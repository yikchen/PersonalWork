use std::env;

#[link(name = "our_code")]
extern "C" {
    // The \x01 here is an undocumented feature of LLVM that ensures
    // it does not add an underscore in front of the name.
    // Courtesy of Max New (https://maxsnew.com/teaching/eecs-483-fa22/hw_adder_assignment.html)
    #[link_name = "\x01our_code_starts_here"]
    fn our_code_starts_here(input: u64, memory: *mut i64) -> u64;
}

#[export_name = "\x01snek_error"]
pub extern "C" fn snek_error(errcode: i64) {
    // TODO: print error message according to writeup
    if errcode == 0 {
        eprintln!("an error ocurred::overflow");
    }
    else if errcode == 1{
        eprintln!("an error ocurred::invalid argument");
    }
    else if errcode == 2 {
        eprintln!("an error ocurred::index out of bounds");
    }
    std::process::exit(1);
}

fn snek_str(val:u64, seen: &mut Vec<u64>, outer:bool) -> String {
    if val == 7 { "true".to_string()}
    else if val == 3 {"false".to_string()}
    else if val % 2 == 0 {format!("{}", (val as i64) >>1)}
    else if val == 1 {"nil".to_string()}
    else if val & 1 == 1 {
        if seen.contains(&val) {return "(list <cyclic>)".to_string()}
        seen.push(val);
        let addr = (val - 1) as *const i64;
        let fst = unsafe {*addr};
        let next = unsafe {*addr.offset(1)};
        let mut result = "".to_string();

        if next == 1{
            if fst & 1 == 1 && outer{
                result = format!("(({}))", snek_str(fst as u64 ,seen,false));
            }
            else if fst & 1 == 1 {
                result = format!("({})", snek_str(fst as u64 ,seen,false));
            }
            else if outer {
                result = format!("({})", snek_str(fst as u64 ,seen,false));
            }
            else {
                result = format!("{}", snek_str(fst as u64 ,seen,false));
            }
        }
        else{
            if fst & 1 == 1 && outer{
                result = format!("(({}) {})", snek_str(fst as u64 ,seen,false), snek_str(next as u64,seen,false));
            }
            else if fst & 1 == 1 {
                result = format!("({}) {}", snek_str(fst as u64 ,seen,false), snek_str(next as u64,seen,false));
            }
            else if outer {
                result = format!("({} {})", snek_str(fst as u64 ,seen,false), snek_str(next as u64,seen,false));
            }
            else {
                result = format!("{} {}", snek_str(fst as u64 ,seen,false), snek_str(next as u64,seen,false));
            }
        }
        seen.pop();
        return result;
    }
    else {
        println!("Unknown value: {}", val);
        std::process::exit(1);
    }
}


unsafe fn snek_eq_helper(val1:u64, val2:u64, seen1: &mut Vec<u64>, seen2: &mut Vec<u64>) -> i64{
    if seen1.contains(&val1) && seen2.contains(&val2){
        let index1 = seen1.iter().position(|&r| r == val1).unwrap();
        let index2 = seen2.iter().position(|&r| r == val2).unwrap();
        if index1 == index2{
            return 7;
        }
        else{
            return 3;
        }
    }
    else if !seen1.contains(&val1) && !seen2.contains(&val2){
        seen1.push(val1);
        seen2.push(val2);

        if val1 == val2 {
            return 7;
        }
        else if val1 &1  == 1 && val2 & 1 == 1{
            let addr1 = (val1 - 1) as *const i64;
            let addr2 = (val2 - 1) as *const i64;
            let v1 = addr1.read() as u64;
            let v2 = addr2.read() as u64;
            let v1_n = addr1.add(1).read() as u64;
            let v2_n = addr2.add(1).read() as u64;

            let result1 = snek_eq_helper(v1,v2,seen1,seen2);
            let result2 = snek_eq_helper(v1_n,v2_n,seen1,seen2);
            if result1 == 7 && result2 == 7{
                return 7;
            }
            else{
                return 3;
            }
        }
        else{
            return 3;
        }
    }
    else{
        return 3;
    }
    
}

#[export_name = "\x01snek_print"]
pub extern "C" fn snek_print(val: u64) -> i64{
    let mut seen = Vec::<u64>::new();
    println!("{}", snek_str(val, &mut seen, true));
    return val as i64;
}


#[export_name = "\x01snek_eq"]
pub unsafe extern "C" fn snek_eq(val1: u64, val2: u64) -> i64{
    let mut seen1 = Vec::<u64>::new();
    let mut seen2 = Vec::<u64>::new();

    let addr = (val1 - 1) as *const i64;
    let value = snek_eq_helper(val1,val2, &mut seen1, &mut seen2);
    return value;
} 

fn parse_input(input: &str) -> u64 {
    // TODO: parse the input string into internal value representation
    if input == "true" {
        7
    }
    else if input == "false" {
        3
    }
    else if input == "nil"{
        1
    }
    else{
        let num = input.parse::<i64>().unwrap();
        if num > -4611686018427387904 && num <= 4611686018427387903 {
            (input.parse::<i64>().unwrap() << 1) as u64
        }
        else{
            println!("invalid argument:: overflow");
            std::process::exit(1);
        }
    }
}

fn main() {
    let args: Vec<String> = env::args().collect();
    let input = if args.len() == 2 { &args[1] } else { "false" };
    let input = parse_input(&input);

    let mut memory = Vec::<i64>::with_capacity(1000000);
    let buffer :*mut i64 = memory.as_mut_ptr();

    let i: u64 = unsafe { our_code_starts_here(input, buffer) };
    snek_print(i);
}

