use std::{collections::HashSet, env};

type SnekVal = u64;

#[derive(Clone, Copy, PartialEq, Eq)]
#[repr(i64)]
pub enum ErrCode {
    InvalidArgument = 1,
    Overflow = 2,
    IndexOutOfBounds = 3,
    InvalidVecSize = 4,
    OutOfMemory = 5,
}

const TRUE: u64 = 7;
const FALSE: u64 = 3;

static mut HEAP_START: *const u64 = std::ptr::null();
static mut HEAP_END: *const u64 = std::ptr::null();

#[link(name = "our_code")]
extern "C" {
    // The \x01 here is an undocumented feature of LLVM that ensures
    // it does not add an underscore in front of the name.
    // Courtesy of Max New (https://maxsnew.com/teaching/eecs-483-fa22/hw_adder_assignment.html)
    #[link_name = "\x01our_code_starts_here"]
    fn our_code_starts_here(input: u64, heap_start: *const u64, heap_end: *const u64) -> u64;
}

#[export_name = "\x01snek_error"]
pub extern "C" fn snek_error(errcode: i64) {
    if errcode == ErrCode::InvalidArgument as i64 {
        eprintln!("invalid argument");
    } else if errcode == ErrCode::Overflow as i64 {
        eprintln!("overflow");
    } else if errcode == ErrCode::IndexOutOfBounds as i64 {
        eprintln!("index out of bounds");
    } else if errcode == ErrCode::InvalidVecSize as i64 {
        eprintln!("vector size must be non-negative");
    } else {
        eprintln!("an error ocurred {}", errcode);
    }
    std::process::exit(errcode as i32);
}

#[export_name = "\x01snek_print"]
pub unsafe extern "C" fn snek_print(val: SnekVal) -> SnekVal {
    println!("{}", snek_str(val, &mut HashSet::new()));
    val
}

/// This function is called when the program needs to allocate `count` words of memory and there's no
/// space left. The function should try to clean up space by triggering a garbage collection. If there's
/// not enough space to hold `count` words after running the garbage collector, the program should terminate
/// with an `out of memory` error.
///
/// Args:
///     * `count`: The number of words the program is trying to allocate, including an extra word for
///       the size of the vector and an extra word to store metadata for the garbage collector, e.g.,
///       to allocate a vector of size 5, `count` will be 7.
///     * `heap_ptr`: The current position of the heap pointer (i.e., the value stored in `%r15`). It
///       is guaranteed that `heap_ptr + 8 * count > HEAP_END`, i.e., this function is only called if
///       there's not enough space to allocate `count` words.
///     * `stack_base`: A pointer to the "base" of the stack.
///     * `curr_rbp`: The value of `%rbp` in the stack frame that triggered the allocation.
///     * `curr_rsp`: The value of `%rsp` in the stack frame that triggered the allocation.
///
/// Returns:
///
/// The new heap pointer where the program should allocate the vector (i.e., the new value of `%r15`)
///
#[export_name = "\x01snek_try_gc"]
pub unsafe fn snek_try_gc(
    count: isize,
    heap_ptr: *const u64,
    stack_base: *const u64,
    curr_rbp: *const u64,
    curr_rsp: *const u64,
) -> *const u64 {
    
    let new_ptr = snek_gc(heap_ptr,stack_base,curr_rbp,curr_rsp);
    if new_ptr.add(count as usize) > HEAP_END{
        eprintln!("out of memory");
        std::process::exit(ErrCode::OutOfMemory as i32)
    }
    else {
        new_ptr
    }
}

/// This function should trigger garbage collection and return the updated heap pointer (i.e., the new
/// value of `%r15`). See [`snek_try_gc`] for a description of the meaning of the arguments.
#[export_name = "\x01snek_gc"]
pub unsafe fn snek_gc(
    heap_ptr: *const u64,
    stack_base: *const u64,
    curr_rbp: *const u64,
    curr_rsp: *const u64,
) -> *const u64 {
    let mut ptr = stack_base;
    let mut root_set = Vec::new();
    while ptr >= curr_rsp{ // populate the root set
        let val = *ptr;
        if val != FALSE && val != TRUE && val != 1 && val & 1 == 1 && val >= (HEAP_START as u64) && val <(HEAP_END as u64){
            root_set.push(val);
        }
        ptr = ptr.sub(1);
    }
    mark(root_set.clone());
    fwd_headers(heap_ptr);
    fwd_internal(root_set, stack_base,curr_rsp);

    let mut start_ptr = HEAP_START;
    let mut new_end_ptr = HEAP_START;
    while start_ptr < heap_ptr{ //compact
        let size = start_ptr.add(1).read() as usize;
        if start_ptr.read() != 0{
            let fwded = (start_ptr.read() - 1) as *const u64;
            (fwded.add(1) as *mut u64).write(size as u64);
            for i in 0..size{
                let elem = start_ptr.add(2 + i).read();
                (fwded.add(2 + i) as *mut u64).write(elem);
            }
            (fwded as *mut u64).write(0);
            new_end_ptr = new_end_ptr.add((2+ size) as usize);
        }
        start_ptr = start_ptr.add((2+ size) as usize);
    }
   
    new_end_ptr
}

pub unsafe fn mark(roots:Vec<SnekVal>){
    for refs in roots.iter(){
        mark_heap(*refs);
    }
}

pub unsafe fn mark_heap(r:SnekVal){
    let addr = (r - 1) as *const u64;
    let marked = addr.read() as SnekVal;
    if marked == 1 {
        return
    }
    (addr as *mut u64).write(1);
    let size = addr.add(1).read() as usize;
    for i in 0..size {
        let elem = addr.add(2 + i).read();
        if elem != FALSE && elem != TRUE && elem != 1 && elem & 1 == 1{
            mark_heap(elem);
        }
    }
}

pub unsafe fn fwd_headers(heap_ptr: *const u64){
    let mut from_ptr = HEAP_START;
    let mut to_ptr = HEAP_START;

    while from_ptr < heap_ptr{
        let size = from_ptr.add(1).read();
        if from_ptr.read() == 1{
            (from_ptr as *mut u64).write(to_ptr as u64);
            to_ptr = to_ptr.add((2 + size) as usize);
        }
        from_ptr = from_ptr.add((2 + size) as usize);
    }
}

pub unsafe fn fwd_internal(roots:Vec<SnekVal>, stack_base: *const u64,curr_rsp: *const u64){
    fwd_stack(stack_base, curr_rsp);
    for refs in roots.iter(){
        fwd_heap(*refs);
    }
}

pub unsafe fn fwd_heap(r:SnekVal){
    let addr = (r - 1) as *const u64; //starting address of the data
    let fwded = addr.read() as SnekVal; //read metadata
    if fwded & 1 == 1{
        return
    } 
    (addr as *mut u64).write(fwded + 1);
    let size = addr.add(1).read() as usize;
    for i in 0..size {
        
        let elem = addr.add(2 + i).read();
        if elem != FALSE && elem != TRUE && elem != 1 && elem & 1 == 1{
            let r_n = (elem - 1) as *const u64;
            if r_n.read() & 1 == 1{
                (addr.add(2 + i) as *mut u64).write(r_n.read() as SnekVal);
            }
            else{
                (addr.add(2 + i) as *mut u64).write((r_n.read()) + 1 as SnekVal);
            }
            fwd_heap(elem);
        }
    }
}

pub unsafe fn fwd_stack(stack_base: *const u64,curr_rsp: *const u64){
    let mut ptr = stack_base;
    while ptr >= curr_rsp {
        let val = *ptr;
        if val != FALSE && val != TRUE && val != 1 && val & 1 == 1 && val >= (HEAP_START as u64) && val <(HEAP_END as u64){
            let addr = (val - 1) as *const u64;
            let for_addr = addr.read() + 1;
            (ptr as *mut u64).write(for_addr);
        }
        ptr = ptr.sub(1);
    }
}

/// A helper function that can called with the `(snek-printstack)` snek function. It prints the stack
/// See [`snek_try_gc`] for a description of the meaning of the arguments.
#[export_name = "\x01snek_print_stack"]
pub unsafe fn snek_print_stack(stack_base: *const u64, curr_rbp: *const u64, curr_rsp: *const u64) {
    let mut ptr = stack_base;
    println!("-----------------------------------------");
    while ptr >= curr_rsp {
        let val = *ptr;
        println!("{ptr:?}: {:#0x}", val);
        ptr = ptr.sub(1);
    }
    println!("-----------------------------------------");
}

unsafe fn snek_str(val: SnekVal, seen: &mut HashSet<SnekVal>) -> String {
    if val == TRUE {
        format!("true")
    } else if val == FALSE {
        format!("false")
    } else if val & 1 == 0 {
        format!("{}", (val as i64) >> 1)
    } else if val == 1 {
        format!("nil")
    } else if val & 1 == 1 {
        if !seen.insert(val) {
            return "[...]".to_string();
        }
        let addr = (val - 1) as *const u64;
        let size = addr.add(1).read() as usize;
        let mut res = "[".to_string();
        for i in 0..size {
            let elem = addr.add(2 + i).read();
            res = res + &snek_str(elem, seen);
            if i < size - 1 {
                res = res + ", ";
            }
        }
        seen.remove(&val);
        res + "]"
    } else {
        format!("unknown value: {val}")
    }
}

fn parse_input(input: &str) -> u64 {
    match input {
        "true" => TRUE,
        "false" => FALSE,
        _ => (input.parse::<i64>().unwrap() << 1) as u64,
    }
}

fn parse_heap_size(input: &str) -> usize {
    input.parse::<usize>().unwrap()
}

fn main() {
    let args: Vec<String> = env::args().collect();
    let input = if args.len() >= 2 { &args[1] } else { "false" };
    let heap_size = if args.len() >= 3 { &args[2] } else { "10000" };
    let input = parse_input(&input);
    let heap_size = parse_heap_size(&heap_size);

    // Initialize heap
    let mut heap: Vec<u64> = Vec::with_capacity(heap_size);
    unsafe {
        HEAP_START = heap.as_mut_ptr();
        HEAP_END = HEAP_START.add(heap_size);
    }

    let i: u64 = unsafe { our_code_starts_here(input, HEAP_START, HEAP_END) };
    unsafe { snek_print(i) };
}
