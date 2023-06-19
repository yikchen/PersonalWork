
use std::env;
use std::fs::File;
use std::io::prelude::*;

use std::collections::HashSet;
use sexp::Atom::*;
use sexp::*;

use im::{hashmap, HashMap};

#[derive(Debug)]
enum Val {
    Reg(Reg),
    Imm(i64),
    RegOffset(Reg, i32),
}

#[derive(Debug)]
enum Reg {
    RAX,
    RBX,
    RSP,
    RDI,
    RDX,
    R15,
    RSI,
}

#[derive(Debug)]
enum Instr {
    IMov(Val, Val),
    IAdd(Val, Val),
    ISub(Val, Val),
    IMul(Val, Val),
    ISar(Val, Val),
    IAnd(Val, Val),
    IOr(Val,Val),
    ICmp(Val, Val),
    ICmov(Val, Val),
    ICmovne(Val, Val),
    ICmovo(Val,Val),
    ICmovg(Val, Val),
    ICmovge(Val, Val),
    ICmovl(Val, Val),
    ICmovle(Val, Val),
    IXor(Val, Val),
    ITest(Val, Val),
    
    ILabel(String),
    IJmp(String),
    IJe(String),
    IJne(String),
    IJo(String),

    //news
    ICall(String),
}

#[derive(Debug)]
enum Op1 {
    Add1,
    Sub1,
    IsNum,
    IsBool,
    Print,
}

#[derive(Debug)]
enum Op2 {
    Plus,
    Minus,
    Times,
    Equal,
    Greater,
    GreaterEqual,
    Less,
    LessEqual,
    Index,
    Eq,
}

struct Program {
    defs: Vec<Definition>,
    main: Expr, 
}

#[derive(Debug)]
enum Definition {
    Fun(String, Vec<String>, Box<Expr>),
}
use Definition::*;

#[derive(Debug)]
enum Expr {
    Number(i64),
    Boolean(bool),
    Id(String),
    Let(Vec<(String, Expr)>, Box<Expr>),
    UnOp(Op1, Box<Expr>),
    BinOp(Op2, Box<Expr>, Box<Expr>),
    If(Box<Expr>, Box<Expr>, Box<Expr>),
    Loop(Box<Expr>),
    Break(Box<Expr>),
    Set(String, Box<Expr>),
    Block(Vec<Expr>),
    Call(String, Vec<Expr>),
    Tuples(Vec<Expr>),
    Vset(Box<Expr>, Box<Expr>,Box<Expr>),
}

fn parse_bind(s: &Sexp) -> (String, Expr){
    match s {
        Sexp::List(vec) =>{
            let restricted = ["add1","sub1","let", "+", "-", "*", "=", "<", "<=", ">",">=", "true","false", "block", "loop", "break", "if", "set!","isnum","isbool","input","print","fun", "tuples", "vec-set!"];
            match &vec[..] {
                [Sexp::Atom(S(n)),e] if !restricted.iter().any(|&w| w == n.to_string()) => (n.to_string(), parse_expr(e)),
                _ => panic!("Invalid_keyword"),
            } 
        },
        _ => panic!("Invalid"),
    }
}

//------------------------- new parse for func
fn is_def(s: &Sexp) -> bool {
    match s {
        Sexp::List(def_vec) => match &def_vec[..] {
            [Sexp::Atom(S(keyword)), Sexp::List(_), _] if keyword == "fun" => {  true},
            _ => {  false}
        }
        _ => false,
    }
}

fn parse_definition(s: &Sexp) -> Definition {
    match s {
        Sexp::List(def_vec) => match &def_vec[..] {
            [Sexp::Atom(S(keyword)), Sexp::List(name_vec), body] if keyword == "fun" => {
                if name_vec.is_empty(){
                    panic!("Invalid no function name");
                }
                let mut arg_vec = Vec::new();
                let mut funcname = "".to_string();
                let mut i = 0;
                let restricted = ["add1","sub1","let", "+", "-", "*", "=", "<", "<=", ">",">=", "true","false", "block", "loop", "break", "if", "set!","isnum","isbool","input","print","fun"];
                for elem in name_vec.iter() {
                    match elem {
                        
                        Sexp::Atom(S(name)) => {
                            if restricted.iter().any(|&w| w == name.to_string()) {
                                panic!("Invalid Invalid_keyword in arg")
                            }
                            if i == 0{
                                funcname.push_str(name);
                            }
                            else{
                                arg_vec.push(name.to_string());
                            }
                            i += 1;
                        },
                        _ => panic!("Invalid Sexp::List in definition name"),
                    }
                    
                }
                Fun(funcname, arg_vec, Box::new(parse_expr(body)))
            },
            _ => panic!("Invalid Bad fundef"),
        },
        _ => panic!("Invalid Bad fundef"),
    }
}

fn parse_program(s: &Sexp) -> Program {
    match s {
        Sexp::List(vec) => {
            let mut defs: Vec<Definition> = vec![];
            let mut i = 0;
            for def_or_exp in vec {
                if is_def(def_or_exp) {
                    defs.push(parse_definition(def_or_exp));
                } else if i == (vec.len() -1) as i32 {
                    return Program {
                        defs: defs,
                        main: parse_expr(def_or_exp),
                    };
                }
                else{
                    panic!("Invalid");
                }
                i+=1;
            }
            panic!("Invalid");
        }
        _ => panic!("Invalid Program should be a list")
    }
}

//-------------------------

fn parse_expr(s: &Sexp) -> Expr{
    match s {
        Sexp::Atom(I(n)) => Expr::Number(i64::try_from(*n).unwrap()),
        Sexp::Atom(S(n)) if n == "true" => Expr::Boolean(true),
        Sexp::Atom(S(n)) if n == "false" => Expr::Boolean(false),
        Sexp::Atom(S(n)) => Expr::Id(n.to_string()),
        Sexp::List(vec) => {
            match &vec[..] {
                [Sexp::Atom(S(op)),e] if op == "add1" => Expr::UnOp(Op1::Add1, Box::new(parse_expr(e))),
                [Sexp::Atom(S(op)),e] if op == "sub1" => Expr::UnOp(Op1::Sub1, Box::new(parse_expr(e))),
                [Sexp::Atom(S(op)),e] if op == "isnum" => Expr::UnOp(Op1::IsNum, Box::new(parse_expr(e))),
                [Sexp::Atom(S(op)),e] if op == "isbool" => Expr::UnOp(Op1::IsBool, Box::new(parse_expr(e))),
                [Sexp::Atom(S(op)),e] if op == "print" => Expr::UnOp(Op1::Print, Box::new(parse_expr(e))),
                [Sexp::Atom(S(op)),e] if op == "loop" => Expr::Loop(Box::new(parse_expr(e))),
                [Sexp::Atom(S(op)),e] if op == "break" => Expr::Break(Box::new(parse_expr(e))),
                [Sexp::Atom(S(op))] if op == "block" => panic!("Invalid"),
                [Sexp::Atom(S(op)), exprs @ ..] if op == "block" => Expr::Block(exprs.into_iter().map(parse_expr).collect()),
                [Sexp::Atom(S(op)), exprs @ ..] if op == "tuples" => Expr::Tuples(exprs.into_iter().map(parse_expr).collect()),
                [Sexp::Atom(S(op)),Sexp::List(e1),e2] if op == "let" && !e1.is_empty() => Expr::Let(e1.iter().map(|e1| parse_bind(e1)).collect::<Vec<(String,Expr)>>(), Box::new(parse_expr(e2))),
                [Sexp::Atom(S(op)),e1,e2] if op == "index" => Expr::BinOp(Op2::Index, Box::new(parse_expr(e1)), Box::new(parse_expr(e2))),
                [Sexp::Atom(S(op)),e1,e2] if op == "+" => Expr::BinOp(Op2::Plus, Box::new(parse_expr(e1)), Box::new(parse_expr(e2))),
                [Sexp::Atom(S(op)),e1,e2] if op == "-" => Expr::BinOp(Op2::Minus, Box::new(parse_expr(e1)), Box::new(parse_expr(e2))),
                [Sexp::Atom(S(op)),e1,e2] if op == "*" => Expr::BinOp(Op2::Times, Box::new(parse_expr(e1)), Box::new(parse_expr(e2))),
                [Sexp::Atom(S(op)),e1,e2] if op == "=" => Expr::BinOp(Op2::Equal, Box::new(parse_expr(e1)), Box::new(parse_expr(e2))),
                [Sexp::Atom(S(op)),e1,e2] if op == ">" => Expr::BinOp(Op2::Greater, Box::new(parse_expr(e1)), Box::new(parse_expr(e2))),
                [Sexp::Atom(S(op)),e1,e2] if op == ">=" => Expr::BinOp(Op2::GreaterEqual, Box::new(parse_expr(e1)), Box::new(parse_expr(e2))),
                [Sexp::Atom(S(op)),e1,e2] if op == "<" => Expr::BinOp(Op2::Less, Box::new(parse_expr(e1)), Box::new(parse_expr(e2))),
                [Sexp::Atom(S(op)),e1,e2] if op == "<=" => Expr::BinOp(Op2::LessEqual, Box::new(parse_expr(e1)), Box::new(parse_expr(e2))),
                [Sexp::Atom(S(op)),e1,e2] if op == "==" => Expr::BinOp(Op2::Eq, Box::new(parse_expr(e1)), Box::new(parse_expr(e2))),
                [Sexp::Atom(S(op)),Sexp::Atom(S(name)),e] if op == "set!" => Expr::Set(name.to_string(), Box::new(parse_expr(e))),
                [Sexp::Atom(S(op)),e1,e2,e3] if op == "if" => Expr::If(Box::new(parse_expr(e1)), Box::new(parse_expr(e2)), Box::new(parse_expr(e3))),
                [Sexp::Atom(S(op)), e1, e2,e3] if op == "vec-set!" => Expr::Vset(Box::new(parse_expr(e1)), Box::new(parse_expr(e2)), Box::new(parse_expr(e3))),
                [Sexp::Atom(S(funcname)), args @ ..] => Expr::Call(funcname.to_string(), args.into_iter().map(parse_expr).collect()),
                _ => panic!("Invalid parse error"),
            }
        },
        _ => panic!("Invalid parse error"),
    }
}

fn compile_to_instrs(e: &Expr, si:i32, env:&HashMap<String,i32>, func_env:& mut im::HashMap<String,i32>, is_func:bool ,brake:&String ,l: &mut i32) -> Vec<Instr> {
    match e {
        Expr::Number(n) => {
            if n >= &-4611686018427387904 && n <= &4611686018427387903 {
                vec![Instr::IMov(Val::Reg(Reg::RAX),Val::Imm(*n<<1))]
            }
            else{
                panic!("Invalid out side of range");
            }
            
        },
        Expr::Boolean(b) => {
            if *b {
              vec![Instr::IMov(Val::Reg(Reg::RAX), Val::Imm(7))]
            }
            else{
              vec![Instr::IMov(Val::Reg(Reg::RAX), Val::Imm(3))]
            }
        },
        Expr::Id(n) => {
            if n == "input" && is_func {
                panic!("Invalid variable input within function definition");
            }
            else if n == "input" {
                vec![Instr::IMov(Val::Reg(Reg::RAX),Val::Reg(Reg::RDI))]
            }
            else if n == "nil"{
                vec![Instr::IMov(Val::Reg(Reg::RAX),Val::Imm(1))]
            }
            else if env.contains_key(n){
                let instrs = vec![Instr::IMov(Val::Reg(Reg::RAX),Val::RegOffset(Reg::RSP, *env.get(n).unwrap()))];
                instrs

            }
            else {
                panic!("Unbound variable identifier {}", n);
            }
        },
        Expr::UnOp(Op1::Add1, body) => { 
            let mut instrs = compile_to_instrs(body,si,env,func_env,is_func,brake,l);

            instrs.extend(vec![Instr::ITest(Val::Reg(Reg::RAX),Val::Imm(1))]); //test rax, 1
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RBX),Val::Imm(1))]);// mov rbx, 1    invalid_arg
            instrs.extend(vec![Instr::ICmovne(Val::Reg(Reg::RDI), Val::Reg(Reg::RBX))]); //cmovne rdi, rax
            instrs.extend(vec![Instr::IJne("throw_error".to_string())]);

            instrs.extend(vec![Instr::IAdd(Val::Reg(Reg::RAX),Val::Imm(2))]);

            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RBX),Val::Imm(0))]); //mov rbx, 0   overflow
            instrs.extend(vec![Instr::ICmovo(Val::Reg(Reg::RDI), Val::Reg(Reg::RBX))]); // cmovo rdi, rbx
            instrs.extend(vec![Instr::IJo("throw_error".to_string())]);
            instrs
        },
        Expr::UnOp(Op1::Sub1, body) => {
            let mut instrs = compile_to_instrs(body,si,env,func_env,is_func,brake,l);

            instrs.extend(vec![Instr::ITest(Val::Reg(Reg::RAX),Val::Imm(1))]); //test rax, 1
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RBX),Val::Imm(1))]);// mov rbx, 1    invalid_arg
            instrs.extend(vec![Instr::ICmovne(Val::Reg(Reg::RDI), Val::Reg(Reg::RBX))]); //cmovne rdi, rax
            instrs.extend(vec![Instr::IJne("throw_error".to_string())]);

            instrs.extend(vec![Instr::ISub(Val::Reg(Reg::RAX),Val::Imm(2))]);

            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RBX),Val::Imm(0))]); //mov rbx, 0   overflow
            instrs.extend(vec![Instr::ICmovo(Val::Reg(Reg::RDI), Val::Reg(Reg::RBX))]); // cmovo rdi, rbx
            instrs.extend(vec![Instr::IJo("throw_error".to_string())]);
            instrs
        },
        Expr::UnOp(Op1::IsNum, body) => {
            let mut instrs = compile_to_instrs(body,si, env,func_env,is_func,brake,l);
            instrs.extend(vec![Instr::IAnd(Val::Reg(Reg::RAX), Val::Imm(1))]);
            instrs.extend(vec![Instr::ICmp(Val::Reg(Reg::RAX), Val::Imm(0))]);
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RBX),Val::Imm(3))]);
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RAX), Val::Imm(1))]);
            instrs.extend(vec![Instr::ICmov(Val::Reg(Reg::RAX),Val::Reg(Reg::RBX))]);
            instrs
        },
        Expr::UnOp(Op1::IsBool, body) => {
            let mut instrs = compile_to_instrs(body,si, env,func_env,is_func,brake,l);
            instrs.extend(vec![Instr::IAnd(Val::Reg(Reg::RAX), Val::Imm(1))]);
            instrs.extend(vec![Instr::ICmp(Val::Reg(Reg::RAX), Val::Imm(1))]);
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RBX),Val::Imm(3))]);
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RAX), Val::Imm(1))]);
            instrs.extend(vec![Instr::ICmov(Val::Reg(Reg::RAX),Val::Reg(Reg::RBX))]);
            instrs
        },
        Expr::BinOp(Op2::Plus, e1,e2) =>{
            let stack_offset = si * 8;
            let mut instrs = compile_to_instrs(e1,si,env,func_env,is_func,brake,l);
            instrs.extend(vec![Instr::IMov(Val::RegOffset(Reg::RSP,stack_offset),Val::Reg(Reg::RAX))]);
            
            //error checking if e1 is boolean jmp error
            instrs.extend(vec![Instr::ITest(Val::Reg(Reg::RAX),Val::Imm(1))]); //test rax, 1
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RAX),Val::Imm(1))]);// mov rax, 1    invalid_arg
            instrs.extend(vec![Instr::ICmovne(Val::Reg(Reg::RDI), Val::Reg(Reg::RAX))]); //cmovne rdi, rax
            instrs.extend(vec![Instr::IJne("throw_error".to_string())]);

            instrs.extend(compile_to_instrs(e2,si+1,env,func_env,is_func,brake,l));
            
            //check if e1 and e2 are same type
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RBX),Val::Reg(Reg::RAX))]);// mov rbx, rax
            instrs.extend(vec![Instr::IXor(Val::Reg(Reg::RBX),Val::RegOffset(Reg::RSP, stack_offset))]);// xor rbx, rsp- stack_offset
            instrs.extend(vec![Instr::ITest(Val::Reg(Reg::RBX),Val::Imm(1))]);// test rbx, 1
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RBX),Val::Imm(1))]); //mov rbx, 1  invalid_arg
            instrs.extend(vec![Instr::ICmovne(Val::Reg(Reg::RDI), Val::Reg(Reg::RBX))]); // cmovne, rdi, rbx
            instrs.extend(vec![Instr::IJne("throw_error".to_string())]);

            instrs.extend(vec![Instr::IAdd(Val::Reg(Reg::RAX),Val::RegOffset(Reg::RSP,stack_offset))]);
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RBX),Val::Imm(0))]); //mov rbx, 0   overflow
            instrs.extend(vec![Instr::ICmovo(Val::Reg(Reg::RDI), Val::Reg(Reg::RBX))]); // cmovo rdi, rbx
            instrs.extend(vec![Instr::IJo("throw_error".to_string())]);
            instrs
        },
        
        Expr::BinOp(Op2::Minus, e1,e2) =>{
            let stack_offset = si * 8;

            let mut instrs = compile_to_instrs(e2,si,env,func_env,is_func,brake,l);
            instrs.extend(vec![Instr::IMov(Val::RegOffset(Reg::RSP,stack_offset),Val::Reg(Reg::RAX))]);

            //error checking if e1 is boolean jmp error
            instrs.extend(vec![Instr::ITest(Val::Reg(Reg::RAX),Val::Imm(1))]); //test rax, 1
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RAX),Val::Imm(1))]);// mov rax, 1    invalid_arg
            instrs.extend(vec![Instr::ICmovne(Val::Reg(Reg::RDI), Val::Reg(Reg::RAX))]); //cmovne rdi, rax
            instrs.extend(vec![Instr::IJne("throw_error".to_string())]);

            instrs.extend(compile_to_instrs(e1,si+1,env,func_env,is_func,brake,l));
            
            //check if e1 and e2 are same type
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RBX),Val::Reg(Reg::RAX))]);// mov rbx, rax
            instrs.extend(vec![Instr::IXor(Val::Reg(Reg::RBX),Val::RegOffset(Reg::RSP, stack_offset))]);// xor rbx, rsp- stack_offset
            instrs.extend(vec![Instr::ITest(Val::Reg(Reg::RBX),Val::Imm(1))]);// test rbx, 1
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RBX),Val::Imm(1))]); //mov rbx, 1   invalid_arg
            instrs.extend(vec![Instr::ICmovne(Val::Reg(Reg::RDI), Val::Reg(Reg::RBX))]); // cmovne, rdi, rbx
            instrs.extend(vec![Instr::IJne("throw_error".to_string())]);

            instrs.extend(vec![Instr::ISub(Val::Reg(Reg::RAX),Val::RegOffset(Reg::RSP,stack_offset))]);
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RBX),Val::Imm(0))]); //mov rbx, 0
            instrs.extend(vec![Instr::ICmovo(Val::Reg(Reg::RDI), Val::Reg(Reg::RBX))]); // cmovo rdi, rbx
            instrs.extend(vec![Instr::IJo("throw_error".to_string())]);
            instrs
        },

        Expr::BinOp(Op2::Times, e1,e2) =>{
            let stack_offset = si * 8;
            let mut instrs = compile_to_instrs(e1,si,env,func_env,is_func,brake, l);
            instrs.extend(vec![Instr::IMov(Val::RegOffset(Reg::RSP,stack_offset),Val::Reg(Reg::RAX))]);

            //error checking if e1 is boolean jmp error
            instrs.extend(vec![Instr::ITest(Val::Reg(Reg::RAX),Val::Imm(1))]); //test rax, 1
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RAX),Val::Imm(1))]);// mov rax, 1 invalid_arg
            instrs.extend(vec![Instr::ICmovne(Val::Reg(Reg::RDI), Val::Reg(Reg::RAX))]); //cmovne rdi, rax
            instrs.extend(vec![Instr::IJne("throw_error".to_string())]);

            instrs.extend(compile_to_instrs(e2,si+1,env,func_env,is_func,brake, l));

            //check if e1 and e2 are same type
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RBX),Val::Reg(Reg::RAX))]);// mov rbx, rax
            instrs.extend(vec![Instr::IXor(Val::Reg(Reg::RBX),Val::RegOffset(Reg::RSP, stack_offset))]);// xor rbx, rsp- stack_offset
            instrs.extend(vec![Instr::ITest(Val::Reg(Reg::RBX),Val::Imm(1))]);// test rbx, 1
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RBX),Val::Imm(1))]); //mov rbx, 1 invalid_arg
            instrs.extend(vec![Instr::ICmovne(Val::Reg(Reg::RDI), Val::Reg(Reg::RBX))]); // cmovne, rdi, rbx
            instrs.extend(vec![Instr::IJne("throw_error".to_string())]);

            instrs.extend(vec![Instr::IMul(Val::Reg(Reg::RAX),Val::RegOffset(Reg::RSP,stack_offset))]);
            instrs.extend(vec![Instr::ISar(Val::Reg(Reg::RAX),Val::Imm(1))]);
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RBX),Val::Imm(0))]); //mov rbx, 0 overflow
            instrs.extend(vec![Instr::ICmovo(Val::Reg(Reg::RDI), Val::Reg(Reg::RBX))]); // cmovo rdi, rbx
            instrs.extend(vec![Instr::IJo("throw_error".to_string())]);
            instrs
        },

        Expr::BinOp(Op2::Equal, e1,e2) => {
            let stack_offset = si * 8;
            let mut instrs = compile_to_instrs(e1,si,env,func_env,is_func,brake,l);
            let start_eq = new_label(l, "eq_start");
            instrs.extend(vec![Instr::IMov(Val::RegOffset(Reg::RSP,stack_offset),Val::Reg(Reg::RAX))]);
            instrs.extend(compile_to_instrs(e2,si+1,env,func_env,is_func,brake,l));
            
            //check if less significant bit is 0
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RBX),Val::Reg(Reg::RAX)),
                                Instr::IOr(Val::Reg(Reg::RBX), Val::RegOffset(Reg::RSP, stack_offset)),
                                Instr::ITest(Val::Reg(Reg::RBX), Val::Imm(1)),
                                Instr::IJe(start_eq.clone())
            ]);

            //error checking part
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RBX),Val::Reg(Reg::RAX))]);// mov rbx, rax
            instrs.extend(vec![Instr::IXor(Val::Reg(Reg::RBX),Val::RegOffset(Reg::RSP, stack_offset))]);// xor rbx, rsp-stack_offset
            instrs.extend(vec![Instr::ITest(Val::Reg(Reg::RBX),Val::Imm(3))]); // test rbx, 3
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RBX),Val::Imm(1))]); //mov rbx, 1  invalid_arg
            instrs.extend(vec![Instr::ICmovne(Val::Reg(Reg::RDI), Val::Reg(Reg::RBX))]); // cmovne, rdi, rbx
            instrs.extend(vec![Instr::IJne("throw_error".to_string())]);
            
            instrs.extend(vec![Instr::ILabel(start_eq.clone())]);
            instrs.extend(vec![Instr::ICmp(Val::Reg(Reg::RAX), Val::RegOffset(Reg::RSP, stack_offset))]);
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RBX),Val::Imm(7))]);
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RAX),Val::Imm(3))]);
            instrs.extend(vec![Instr::ICmov(Val::Reg(Reg::RAX),Val::Reg(Reg::RBX))]);
            instrs
        },

        Expr::BinOp(Op2::Greater, e1,e2) => {
            let stack_offset = si * 8;
            let mut instrs = compile_to_instrs(e1,si,env,func_env,is_func,brake,l);
            instrs.extend(vec![Instr::IMov(Val::RegOffset(Reg::RSP,stack_offset),Val::Reg(Reg::RAX))]);
            
            //error checking if e1 is boolean jmp error
            instrs.extend(vec![Instr::ITest(Val::Reg(Reg::RAX),Val::Imm(1))]); //test rax, 1
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RAX),Val::Imm(1))]);// mov rax, 1
            instrs.extend(vec![Instr::ICmovne(Val::Reg(Reg::RDI), Val::Reg(Reg::RAX))]); //cmovne rdi, rax
            instrs.extend(vec![Instr::IJne("throw_error".to_string())]);
            
            instrs.extend(compile_to_instrs(e2,si+1,env,func_env,is_func,brake,l));
            
            //check if e1 and e2 are same type
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RBX),Val::Reg(Reg::RAX))]);// mov rbx, rax
            instrs.extend(vec![Instr::IXor(Val::Reg(Reg::RBX),Val::RegOffset(Reg::RSP, stack_offset))]);// xor rbx, rsp- stack_offset
            instrs.extend(vec![Instr::ITest(Val::Reg(Reg::RBX),Val::Imm(1))]);// test rbx, 1
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RBX),Val::Imm(1))]); //mov rbx, 1
            instrs.extend(vec![Instr::ICmovne(Val::Reg(Reg::RDI), Val::Reg(Reg::RBX))]); // cmovne, rdi, rbx
            instrs.extend(vec![Instr::IJne("throw_error".to_string())]);
            
            instrs.extend(vec![Instr::ICmp(Val::Reg(Reg::RAX), Val::RegOffset(Reg::RSP, stack_offset))]);
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RBX),Val::Imm(7))]);
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RAX),Val::Imm(3))]);
            instrs.extend(vec![Instr::ICmovl(Val::Reg(Reg::RAX),Val::Reg(Reg::RBX))]);
            instrs
        },

        Expr::BinOp(Op2::GreaterEqual, e1,e2) =>{
            let stack_offset = si * 8;
            let mut instrs = compile_to_instrs(e1,si,env,func_env,is_func,brake,l);
            instrs.extend(vec![Instr::IMov(Val::RegOffset(Reg::RSP,stack_offset),Val::Reg(Reg::RAX))]);
            
            //error checking if e1 is boolean jmp error
            instrs.extend(vec![Instr::ITest(Val::Reg(Reg::RAX),Val::Imm(1))]); //test rax, 1
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RAX),Val::Imm(1))]);// mov rax, 1 invalid_arg
            instrs.extend(vec![Instr::ICmovne(Val::Reg(Reg::RDI), Val::Reg(Reg::RAX))]); //cmovne rdi, rax
            instrs.extend(vec![Instr::IJne("throw_error".to_string())]);
            
            
            instrs.extend(compile_to_instrs(e2,si+1,env,func_env,is_func,brake,l));

            //check if e1 and e2 are same type
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RBX),Val::Reg(Reg::RAX))]);// mov rbx, rax
            instrs.extend(vec![Instr::IXor(Val::Reg(Reg::RBX),Val::RegOffset(Reg::RSP, stack_offset))]);// xor rbx, rsp- stack_offset
            instrs.extend(vec![Instr::ITest(Val::Reg(Reg::RBX),Val::Imm(1))]);// test rbx, 1
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RBX),Val::Imm(1))]); //mov rbx, 1 invalid_arg
            instrs.extend(vec![Instr::ICmovne(Val::Reg(Reg::RDI), Val::Reg(Reg::RBX))]); // cmovne, rdi, rbx
            instrs.extend(vec![Instr::IJne("throw_error".to_string())]);

            instrs.extend(vec![Instr::ICmp(Val::Reg(Reg::RAX), Val::RegOffset(Reg::RSP, stack_offset))]);
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RBX),Val::Imm(7))]);
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RAX),Val::Imm(3))]);
            instrs.extend(vec![Instr::ICmovle(Val::Reg(Reg::RAX),Val::Reg(Reg::RBX))]);
            instrs
        },

        Expr::BinOp(Op2::Less, e1,e2) => {
            let stack_offset = si * 8;
            let mut instrs = compile_to_instrs(e1,si,env,func_env,is_func,brake,l);
            instrs.extend(vec![Instr::IMov(Val::RegOffset(Reg::RSP,stack_offset),Val::Reg(Reg::RAX))]);
            
            //error checking if e1 is boolean jmp error
            instrs.extend(vec![Instr::ITest(Val::Reg(Reg::RAX),Val::Imm(1))]); //test rax, 1
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RAX),Val::Imm(1))]);// mov rax, 1 invalid_arg
            instrs.extend(vec![Instr::ICmovne(Val::Reg(Reg::RDI), Val::Reg(Reg::RAX))]); //cmovne rdi, rax
            instrs.extend(vec![Instr::IJne("throw_error".to_string())]);

            instrs.extend(compile_to_instrs(e2,si+1,env,func_env,is_func,brake,l));
            
            //check if e1 and e2 are same type
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RBX),Val::Reg(Reg::RAX))]);// mov rbx, rax
            instrs.extend(vec![Instr::IXor(Val::Reg(Reg::RBX),Val::RegOffset(Reg::RSP, stack_offset))]);// xor rbx, rsp- stack_offset
            instrs.extend(vec![Instr::ITest(Val::Reg(Reg::RBX),Val::Imm(1))]);// test rbx, 1
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RBX),Val::Imm(1))]); //mov rbx, 1 invalid_arg
            instrs.extend(vec![Instr::ICmovne(Val::Reg(Reg::RDI), Val::Reg(Reg::RBX))]); // cmovne, rdi, rbx
            instrs.extend(vec![Instr::IJne("throw_error".to_string())]);

            instrs.extend(vec![Instr::ICmp(Val::Reg(Reg::RAX), Val::RegOffset(Reg::RSP, stack_offset))]);
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RBX),Val::Imm(7))]);
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RAX),Val::Imm(3))]);
            instrs.extend(vec![Instr::ICmovg(Val::Reg(Reg::RAX),Val::Reg(Reg::RBX))]);
            instrs
        },

        Expr::BinOp(Op2::LessEqual, e1,e2) => {
            let stack_offset = si * 8;
            let mut instrs = compile_to_instrs(e1,si,env,func_env,is_func,brake,l);
            instrs.extend(vec![Instr::IMov(Val::RegOffset(Reg::RSP,stack_offset),Val::Reg(Reg::RAX))]);
            
            //error checking if e1 is boolean jmp error
            instrs.extend(vec![Instr::ITest(Val::Reg(Reg::RAX),Val::Imm(1))]); //test rax, 1
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RAX),Val::Imm(1))]);// mov rax, 1 invalid_arg
            instrs.extend(vec![Instr::ICmovne(Val::Reg(Reg::RDI), Val::Reg(Reg::RAX))]); //cmovne rdi, rax
            instrs.extend(vec![Instr::IJne("throw_error".to_string())]);
            
            instrs.extend(compile_to_instrs(e2,si+1,env,func_env,is_func,brake,l));
            
            //check if e1 and e2 are same type
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RBX),Val::Reg(Reg::RAX))]);// mov rbx, rax
            instrs.extend(vec![Instr::IXor(Val::Reg(Reg::RBX),Val::RegOffset(Reg::RSP, stack_offset))]);// xor rbx, rsp- stack_offset
            instrs.extend(vec![Instr::ITest(Val::Reg(Reg::RBX),Val::Imm(1))]);// test rbx, 1
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RBX),Val::Imm(1))]); //mov rbx, 1 invalid_arg
            instrs.extend(vec![Instr::ICmovne(Val::Reg(Reg::RDI), Val::Reg(Reg::RBX))]); // cmovne, rdi, rbx
            instrs.extend(vec![Instr::IJne("throw_error".to_string())]);

            instrs.extend(vec![Instr::ICmp(Val::Reg(Reg::RAX), Val::RegOffset(Reg::RSP, stack_offset))]);
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RBX),Val::Imm(7))]);
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RAX),Val::Imm(3))]);
            instrs.extend(vec![Instr::ICmovge(Val::Reg(Reg::RAX),Val::Reg(Reg::RBX))]);
            instrs
        },
        Expr::BinOp(Op2::Index,e1,e2) =>{
            let stack_offset = si * 8;
            let mut instrs = compile_to_instrs(e1,si,env,func_env,is_func,brake,l);
            
            //tag check for e1(which should be heap allocated structure)

            instrs.extend(vec![Instr::ICmp(Val::Reg(Reg::RAX), Val::Imm(1)),
                                Instr::IMov(Val::Reg(Reg::RBX),Val::Imm(1)), //mov rbx, 1  invalid_arg
                                Instr::ICmov(Val::Reg(Reg::RDI), Val::Reg(Reg::RBX)), // cmove, rdi, rbx
                                Instr::IJe("throw_error".to_string()),

                                Instr::IMov(Val::Reg(Reg::RBX),Val::Reg(Reg::RAX)),
                                Instr::IXor(Val::Reg(Reg::RBX), Val::Imm(1)),
                                Instr::ITest(Val::Reg(Reg::RBX),Val::Imm(3)), //test rbx, 3
                                Instr::IMov(Val::Reg(Reg::RBX),Val::Imm(1)), //mov rbx, 1  invalid_arg
                                Instr::ICmovne(Val::Reg(Reg::RDI), Val::Reg(Reg::RBX)), // cmovne, rdi, rbx
                                Instr::IJne("throw_error".to_string())
            ]);

            
            instrs.extend(vec![Instr::ISub(Val::Reg(Reg::RAX),Val::Imm(1)),
                                Instr::IMov(Val::RegOffset(Reg::RSP, stack_offset),Val::Reg(Reg::RAX))]);

            instrs.extend(compile_to_instrs(e2,si + 1,env,func_env,is_func,brake,l));
            
            let start_index = new_label(l, "index");
            let else_index = new_label(l, "else");
            let end_index = new_label(l, "index_end");
            instrs.extend(vec![Instr::ITest(Val::Reg(Reg::RAX),Val::Imm(1)),
                                Instr::IMov(Val::Reg(Reg::RBX),Val::Imm(1)),
                                Instr::ICmovne(Val::Reg(Reg::RDI), Val::Reg(Reg::RBX)),
                                Instr::IJne("throw_error".to_string()),

                                Instr::IMov(Val::Reg(Reg::RBX), Val::RegOffset(Reg::RSP, stack_offset)),
                                Instr::ILabel(start_index.clone()),
                                Instr::ICmp(Val::Reg(Reg::RAX),Val::Imm(0)),
                                Instr::IJe(end_index.clone()),
                                Instr::IJne(else_index.clone()),
                                Instr::IMov(Val::Reg(Reg::RDX), Val::RegOffset(Reg::RBX, 8)),
                                Instr::ICmp(Val::Reg(Reg::RDX), Val::Imm(1)),
                                Instr::IMov(Val::Reg(Reg::RDX),Val::Imm(2)),
                                Instr::ICmov(Val::Reg(Reg::RDI), Val::Reg(Reg::RDX)),
                                Instr::IJe("throw_error".to_string()),
                                Instr::ILabel(else_index.clone()),
                                Instr::IMov(Val::Reg(Reg::RDX), Val::RegOffset(Reg::RBX, 8)),
                                Instr::ICmp(Val::Reg(Reg::RDX), Val::Imm(1)),
                                Instr::IMov(Val::Reg(Reg::RDX), Val::Imm(2)),
                                Instr::ICmov(Val::Reg(Reg::RDI), Val::Reg(Reg::RDX)),
                                Instr::IJe("throw_error".to_string()),
                                Instr::ISub(Val::Reg(Reg::RAX),Val::Imm(2)),
                                Instr::IAdd(Val::Reg(Reg::RBX),Val::Imm(16)),
                                Instr::IJmp(start_index.clone()),
                                
                                Instr::ILabel(end_index.clone()),
                                Instr::IMov(Val::Reg(Reg::RAX), Val::RegOffset(Reg::RBX, 0)),
            ]);
            instrs
        },

        Expr::BinOp(Op2::Eq, e1,e2) =>{
            let mut instrs = compile_to_instrs(e1,si,env,func_env,is_func,brake,l);
            let stack_offset = si * 8;
            //move value of e1 into stack
            instrs.extend(vec![Instr::IMov(Val::RegOffset(Reg::RSP, stack_offset), Val::Reg(Reg::RAX))]);
            
            //tag checck for e1
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RBX),Val::Reg(Reg::RAX)),
                                Instr::IXor(Val::Reg(Reg::RBX), Val::Imm(1)),
                                Instr::ITest(Val::Reg(Reg::RBX),Val::Imm(3)), //test rbx, 3
                                Instr::IMov(Val::Reg(Reg::RBX),Val::Imm(1)), //mov rbx, 1  invalid_arg
                                Instr::ICmovne(Val::Reg(Reg::RDI), Val::Reg(Reg::RBX)), // cmovne, rdi, rbx
                                Instr::IJne("throw_error".to_string())
            ]);
            //evaluate e2
            instrs.extend(compile_to_instrs(e2, si + 1, env, func_env, is_func, brake, l));
            //tag check for e2
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RBX),Val::Reg(Reg::RAX)),
                                Instr::IXor(Val::Reg(Reg::RBX), Val::Imm(1)),
                                Instr::ITest(Val::Reg(Reg::RBX),Val::Imm(3)), //test rbx, 3
                                Instr::IMov(Val::Reg(Reg::RBX),Val::Imm(1)), //mov rbx, 1  invalid_arg
                                Instr::ICmovne(Val::Reg(Reg::RDI), Val::Reg(Reg::RBX)), // cmovne, rdi, rbx
                                Instr::IJne("throw_error".to_string())
            ]);

            let index = if si == 0 { 3} else if si % 2 == 1 {si + 2} else { si + 1};
            let offset = index * 8;
            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RBX),Val::RegOffset(Reg::RSP, stack_offset)),
                                Instr::ISub(Val::Reg(Reg::RSP),Val::Imm(offset.into())),
                                Instr::IMov(Val::RegOffset(Reg::RSP , 0),Val::Reg(Reg::RDI)),
                                Instr::IMov(Val::RegOffset(Reg::RSP , 8),Val::Reg(Reg::RSI)),
                                Instr::IMov(Val::Reg(Reg::RDI),Val::Reg(Reg::RBX)),
                                Instr::IMov(Val::Reg(Reg::RSI),Val::Reg(Reg::RAX)),
                                Instr::ICall("snek_eq".to_string()),
                                Instr::IMov(Val::Reg(Reg::RDI),Val::RegOffset(Reg::RSP, 0)),
                                Instr::IMov(Val::Reg(Reg::RSI),Val::RegOffset(Reg::RSP, 8)),
                                Instr::IAdd(Val::Reg(Reg::RSP),Val::Imm(offset.into())),
            ]);
            instrs
        }

        Expr::Vset(e1, e2,e3) => {

            let stack_offset = si * 8;
            let stack_offset1 = (si + 1) * 8;
            let mut instrs = compile_to_instrs(e1,si,env,func_env,is_func,brake,l);
            
            //tag check for e1(which should be heap allocated structure)
            instrs.extend(vec![Instr::ICmp(Val::Reg(Reg::RAX), Val::Imm(1)),
                                Instr::IMov(Val::Reg(Reg::RBX),Val::Imm(1)), //mov rbx, 1  invalid_arg
                                Instr::ICmov(Val::Reg(Reg::RDI), Val::Reg(Reg::RBX)), // cmove, rdi, rbx
                                Instr::IJe("throw_error".to_string()),

                                Instr::IMov(Val::Reg(Reg::RBX),Val::Reg(Reg::RAX)),
                                Instr::IXor(Val::Reg(Reg::RBX), Val::Imm(1)),
                                Instr::ITest(Val::Reg(Reg::RBX),Val::Imm(3)), //test rbx, 3
                                Instr::IMov(Val::Reg(Reg::RBX),Val::Imm(1)), //mov rbx, 1  invalid_arg
                                Instr::ICmovne(Val::Reg(Reg::RDI), Val::Reg(Reg::RBX)), // cmovne, rdi, rbx
                                Instr::IJne("throw_error".to_string())
            ]);

            //move value of e1 into stack
            instrs.extend(vec![Instr::ISub(Val::Reg(Reg::RAX),Val::Imm(1)),
                                Instr::IMov(Val::RegOffset(Reg::RSP, stack_offset),Val::Reg(Reg::RAX))]);

            //evaluate the value of expr3 and store it into the stack
            instrs.extend(compile_to_instrs(e3,si + 1, env, func_env, is_func, brake, l));
            instrs.extend(vec![Instr::IMov(Val::RegOffset(Reg::RSP, stack_offset1), Val::Reg(Reg::RAX))]);
            
            //evaluate the value of expr2 and iterate over the heap to the indicated position and change the value into expr3(new_value)
            instrs.extend(compile_to_instrs(e2, si + 2, env, func_env, is_func, brake, l));
            
            let start_index = new_label(l, "index");
            let else_index = new_label(l, "else");
            let end_index = new_label(l, "index_end");
            instrs.extend(vec![Instr::ITest(Val::Reg(Reg::RAX),Val::Imm(1)),
                                Instr::IMov(Val::Reg(Reg::RBX),Val::Imm(1)),
                                Instr::ICmovne(Val::Reg(Reg::RDI), Val::Reg(Reg::RBX)),
                                Instr::IJne("throw_error".to_string()),

                                Instr::IMov(Val::Reg(Reg::RBX), Val::RegOffset(Reg::RSP, stack_offset)),
                                Instr::ILabel(start_index.clone()),
                                Instr::ICmp(Val::Reg(Reg::RAX),Val::Imm(0)),
                                Instr::IJe(end_index.clone()),
                                Instr::IJne(else_index.clone()),
                                Instr::IMov(Val::Reg(Reg::RDX), Val::RegOffset(Reg::RBX, 8)),
                                Instr::ICmp(Val::Reg(Reg::RDX), Val::Imm(1)),
                                Instr::IMov(Val::Reg(Reg::RDX),Val::Imm(2)),
                                Instr::ICmov(Val::Reg(Reg::RDI), Val::Reg(Reg::RDX)),
                                Instr::IJe("throw_error".to_string()),
                                Instr::ILabel(else_index.clone()),
                                Instr::IMov(Val::Reg(Reg::RDX), Val::RegOffset(Reg::RBX, 8)),
                                Instr::ICmp(Val::Reg(Reg::RDX), Val::Imm(1)),
                                Instr::IMov(Val::Reg(Reg::RDX), Val::Imm(2)),
                                Instr::ICmov(Val::Reg(Reg::RDI), Val::Reg(Reg::RDX)),
                                Instr::IJe("throw_error".to_string()),
                                Instr::ISub(Val::Reg(Reg::RAX),Val::Imm(2)),
                                Instr::IAdd(Val::Reg(Reg::RBX),Val::Imm(16)),
                                Instr::IJmp(start_index.clone()),
                                
                                Instr::ILabel(end_index.clone()),
                                Instr::IMov(Val::Reg(Reg::RAX), Val::RegOffset(Reg::RSP, stack_offset1)),
                                Instr::IMov(Val::RegOffset(Reg::RBX, 0), Val::Reg(Reg::RAX)),
                                Instr::IMov(Val::Reg(Reg::RAX), Val::RegOffset(Reg::RSP, stack_offset)),
                                Instr::IAdd(Val::Reg(Reg::RAX), Val::Imm(1))
            ]);
            instrs
        },

        Expr::If(e1,e2,e3) =>{
            let end_label = new_label(l,"ifend");
            let else_label = new_label(l, "ifelse");
            let mut instrs = compile_to_instrs(e1,si,env,func_env,is_func,brake,l); //condition
            instrs.extend(vec![Instr::ICmp(Val::Reg(Reg::RAX),Val::Imm(3))]); //cmp rax, 3 
            instrs.extend(vec![Instr::IJe(else_label.clone())]); //je else
            instrs.extend(compile_to_instrs(e2,si,env,func_env,is_func,brake,l));// then instrs
            instrs.extend(vec![Instr::IJmp(end_label.clone())]); // jmp end
            instrs.extend(vec![Instr::ILabel(else_label.clone())]);// else label
            instrs.extend(compile_to_instrs(e3,si,env,func_env,is_func,brake,l));// else instrs
            instrs.extend(vec![Instr::ILabel(end_label.clone())]);// end label
            instrs
        },

        Expr::Block(es) =>{
            es.into_iter().map(|e| { compile_to_instrs(e, si, env, func_env,is_func,brake, l) }).flatten().collect::<Vec<Instr>>()
        },

        Expr::Set(name, e) => {
            if env.contains_key(name){
                let stack_offset = env.get(name).unwrap();
                let mut instrs = compile_to_instrs(e,si,env,func_env,is_func,brake,l);
                instrs.extend(vec![Instr::IMov(Val::RegOffset(Reg::RSP,*stack_offset),Val::Reg(Reg::RAX))]);
                instrs
            }
            else{
                panic!("Unbound variable identifier {}", name);
            }
            
        },

        Expr::Loop(e) => {
            let start_loop = new_label(l, "loop");
            let end_loop = new_label(l, "loop_end");
            let mut instrs = vec![Instr::ILabel(start_loop.clone())];
            instrs.extend(compile_to_instrs(e,si,env,func_env,is_func,&end_loop,l));
            instrs.extend(vec![Instr::IJmp(start_loop.clone())]);
            instrs.extend(vec![Instr::ILabel(end_loop.clone())]);
            instrs
        },

        Expr::Break(e) => {
            if brake == "no_loop" {
                panic!("Invalid break");
            }
            let mut instrs = compile_to_instrs(e,si,env,func_env,is_func,brake,l);
            instrs.extend(vec![Instr::IJmp(brake.to_string())]);
            instrs
        },

        Expr::Let(bindings,body) =>{
            let mut count = si;
            let mut instrs = Vec::new();
            let mut nenv = env.clone();
            let mut ids = Vec::new();
            
            for (id,e) in bindings.iter(){
                
                if !ids.contains(id) {
                    ids.push(id.to_string());
                    let e_instrs = compile_to_instrs(e,count,&nenv,func_env,is_func,brake,l);
                    instrs.extend(e_instrs);
                    instrs.extend(vec![Instr::IMov(Val::RegOffset(Reg::RSP,count*8),Val::Reg(Reg::RAX))]);
                    nenv.insert(id.to_string(),count*8);
                    count +=1;
                }
                else {
                    panic!("Duplicate binding");
                }
            }
            
            instrs.extend(compile_to_instrs(body,count,&nenv,func_env,is_func,brake,l));
            instrs
        },
        Expr::UnOp(Op1::Print, body) =>{
            let mut instrs = compile_to_instrs(body,si,env,func_env,is_func,brake,l);

            let index = if si == 0 { 3} else if si % 2 == 1 {si + 2} else { si + 1};
            let offset = index * 8;
            instrs.extend(vec![Instr::ISub(Val::Reg(Reg::RSP),Val::Imm(offset.into())),
                                Instr::IMov(Val::RegOffset(Reg::RSP , 0),Val::Reg(Reg::RDI)),
                                Instr::IMov(Val::RegOffset(Reg::RSP , 8),Val::Reg(Reg::RAX)),
                                Instr::IMov(Val::Reg(Reg::RDI),Val::Reg(Reg::RAX)),
                                Instr::ICall("snek_print".to_string()),
                                Instr::IMov(Val::Reg(Reg::RDI),Val::RegOffset(Reg::RSP, 0)),
                                Instr::IMov(Val::Reg(Reg::RAX),Val::RegOffset(Reg::RSP, 8)),
                                Instr::IAdd(Val::Reg(Reg::RSP),Val::Imm(offset.into()))
            ]);
            instrs
        },
        Expr::Call(funcname, arg) => {
            if !func_env.contains_key(funcname){
                panic!("Invalid call to non exist function");
            }
            if func_env[funcname] != (arg.len() as i32) {
                panic!("Invalid call to a function with the wrong number of arguments");
            }
            let space = (arg.len()+1) as i32;
            let offset = if space % 2 == 0 {(space) * 8} else {(space+1)*8};
            
            let mut instrs: Vec<Instr> = vec![];
            
            let mut i:i32 = 0;
            for expr in arg.iter() {
                let curr_word = (si + i) * 8;
                instrs.extend(compile_to_instrs(expr, si + i, env, func_env, is_func, brake, l));
                if i+1 < (arg.len() as i32){
                    instrs.extend(vec![Instr::IMov(Val::RegOffset(Reg::RSP, curr_word),Val::Reg(Reg::RAX))]);
                }
                i +=1;
            }
            instrs.extend(vec![Instr::ISub(Val::Reg(Reg::RSP),Val::Imm(offset.into()))]); //sub rsp, offset = even number

            i = 0;
            for _expr in arg.iter() {
                let curr_word_after_sub = offset + (si + i) * 8;
                if i + 1 < (arg.len() as i32){
                    instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RBX),Val::RegOffset(Reg::RSP, curr_word_after_sub)),
                                        Instr::IMov(Val::RegOffset(Reg::RSP, i*8),Val::Reg(Reg::RBX))
                    ]);
                }
                i +=1;
            }
            instrs.extend(vec![Instr::IMov(Val::RegOffset(Reg::RSP, (i - 1)* 8),Val::Reg(Reg::RAX)),
                                Instr::IMov(Val::RegOffset(Reg::RSP, i * 8),Val::Reg(Reg::RDI)),
                                Instr::ICall(funcname.to_string()),
                                Instr::IMov(Val::Reg(Reg::RDI),Val::RegOffset(Reg::RSP, i * 8)),
                                Instr::IAdd(Val::Reg(Reg::RSP),Val::Imm(offset.into()))
            ]);
            instrs
        },

        //news
        Expr::Tuples(exprs) =>{
            let mut instrs:Vec<Instr> = vec![];
            let mut i:i32 = 0;
            for expr in exprs.iter(){
                let curr_offset = (si + i) * 8;
                instrs.extend(compile_to_instrs(expr, si + i, env, func_env, is_func, brake, l));
                if i + 1 < (exprs.len() as i32){
                    instrs.extend(vec![Instr::IMov(Val::RegOffset(Reg::RSP, curr_offset),Val::Reg(Reg::RAX))]);
                }
                i +=1;
            }
            instrs.extend(vec![Instr::IMov(Val::RegOffset(Reg::R15, (i -1)* 2 * 8), Val::Reg(Reg::RAX)),
                                Instr::IMov(Val::Reg(Reg::RAX), Val::Imm(1)),
                                Instr::IMov(Val::RegOffset(Reg::R15, (i -1)* 2 * 8 + 8) , Val::Reg(Reg::RAX)),
                                Instr::IMov(Val::Reg(Reg::RBX),Val::Reg(Reg::R15)),
            ]);

            i = 0;
            for _expr in exprs.iter(){
                let curr_offset = (si + i) * 8;
                let heap_offset = 2*i * 8;
                if i + 1 < (exprs.len() as i32){
                    instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RAX),Val::RegOffset(Reg::RSP, curr_offset)),
                                        Instr::IMov(Val::RegOffset(Reg::R15, heap_offset),Val::Reg(Reg::RAX)),
                                        Instr::IAdd(Val::Reg(Reg::RBX), Val::Imm(16)),
                                        Instr::IMov(Val::RegOffset(Reg::R15, heap_offset + 8),Val::Reg(Reg::RBX)),
                                        Instr::IMov(Val::Reg(Reg::RAX), Val::Imm(1)),
                                        Instr::IAdd(Val::RegOffset(Reg::R15, heap_offset + 8),Val::Reg(Reg::RAX)),
                    ]);
                }
                i += 1;
            }

            instrs.extend(vec![Instr::IMov(Val::Reg(Reg::RAX),Val::Reg(Reg::R15)), 
                                Instr::IAdd(Val::Reg(Reg::RAX),Val::Imm(1)),
                                Instr::IAdd(Val::Reg(Reg::R15), Val::Imm((2*i*8).into()))
            ]);
            instrs
        }
    }
}

fn new_label(l: &mut i32, s: &str) -> String{
    let current = *l;
    *l +=1;
    format!("{}_{}", s,current)
}

fn val_to_str(v: &Val) -> String{
    match v {
        Val::Reg(Reg::RAX) => "rax".to_string(),
        Val::Reg(Reg::RSP) => "rsp".to_string(),
        Val::Reg(Reg::RBX) => "rbx".to_string(),
        Val::Reg(Reg::RDI) => "rdi".to_string(),
        Val::Reg(Reg::RDX) => "rdx".to_string(),
        Val::Reg(Reg::R15) => "r15".to_string(),
        Val::Reg(Reg::RSI) => "rsi".to_string(),
        Val::Imm(n) => n.to_string(),
        Val::RegOffset(Reg::RAX, n) => format!("[RAX + {}]",*n),
        Val::RegOffset(Reg::RSP, n) => format!("[RSP + {}]", *n),
        Val::RegOffset(Reg::RBX, n) => format!("[RBX + {}]", *n),
        Val::RegOffset(Reg::RDI, n) => format!("[RDI + {}]", *n),
        Val::RegOffset(Reg::RDX, n) => format!("[RDX + {}]", *n),
        Val::RegOffset(Reg::RSI, n) => format!("[RSI + {}]", *n),
        Val::RegOffset(Reg::R15, n) => format!("[R15 + {}]", *n),
    }
}

fn instr_to_str(i: &Instr) -> String{
    match i {
        Instr::IMov(v1,v2) => format!("\nmov {}, {}",val_to_str(v1),val_to_str(v2)),
        Instr::IAdd(v1,v2) => format!("\nadd {}, {}",val_to_str(v1),val_to_str(v2)),
        Instr::ISub(v1,v2) => format!("\nsub {}, {}",val_to_str(v1),val_to_str(v2)),
        Instr::IMul(v1,v2) => format!("\nimul {}, {}",val_to_str(v1),val_to_str(v2)),
        Instr::ISar(v1,v2) => format!("\nsar {}, {}", val_to_str(v1),val_to_str(v2)),
        Instr::IAnd(v1,v2) => format!("\nand {}, {}",val_to_str(v1),val_to_str(v2)),
        Instr::IOr(v1,v2) => format!("\nor {}, {}",val_to_str(v1),val_to_str(v2)),
        Instr::ICmp(v1,v2) => format!("\ncmp {}, {}",val_to_str(v1),val_to_str(v2)),
        Instr::ICmov(v1,v2) => format!("\ncmove {}, {}",val_to_str(v1),val_to_str(v2)),
        Instr::ICmovne(v1,v2) => format!("\ncmovne {}, {}",val_to_str(v1),val_to_str(v2)),
        Instr::ICmovo(v1,v2) => format!("\ncmovo {}, {}", val_to_str(v1),val_to_str(v2)),
        Instr::ICmovg(v1,v2) => format!("\ncmovg {}, {}",val_to_str(v1),val_to_str(v2)),
        Instr::ICmovge(v1,v2) => format!("\ncmovge {}, {}",val_to_str(v1),val_to_str(v2)),
        Instr::ICmovl(v1,v2) => format!("\ncmovl {}, {}",val_to_str(v1),val_to_str(v2)),
        Instr::ICmovle(v1,v2) => format!("\ncmovle {}, {}",val_to_str(v1),val_to_str(v2)),
        Instr::ITest(v1,v2) => format!("\ntest {}, {}",val_to_str(v1),val_to_str(v2)),
        Instr::IXor(v1,v2) => format!("\nxor {}, {}",val_to_str(v1),val_to_str(v2)),
        Instr::ILabel(name) => format!("\n{}:", name),
        Instr::IJmp(name) => format!("\njmp {}", name),
        Instr::IJe(name) => format!("\nje {}", name),
        Instr::IJne(name) => format!("\njne {}", name),
        Instr::IJo(name) => format!("\njo {}", name),
        Instr::ICall(funcname) => format!("\ncall {}", funcname),
    }
}

fn compile(e: &Expr,si: i32, env:&HashMap<String,i32>, func_env:& mut im::HashMap<String,i32>, is_func:bool, brake: &String ,l: &mut i32) -> String{
    let instrs = compile_to_instrs(e,si,env,func_env,is_func,brake,l);
    let mut output = "".to_string();
    for instructions in instrs.iter(){
        let temp = instr_to_str(instructions);
        output.push_str(&temp);
    }
    output
}


fn depth(e: &Expr) -> i32 {
    match e {
        Expr::Number(_) => 0,
        Expr::Boolean(_) => 0,
        Expr::UnOp(_,expr) => depth(expr),
        Expr::BinOp(Op2::Minus, expr1, expr2) => depth(expr2).max(depth(expr1) + 1),
        Expr::BinOp(_, expr1, expr2) => depth(expr1).max(depth(expr2) + 1),
        Expr::Let(bindings, body) =>  {
            let mut sum = 0;
            let mut i = 0;
            for (_id,e) in bindings.iter(){
                sum = sum.max(depth(e) + i);
                i += 1;
            }
            
            sum.max(depth(body) + i)
        },
        Expr::Id(_) => 0,
        Expr::If(expr1, expr2, expr3) => depth(expr1).max(depth(expr2)).max(depth(expr3)),
        Expr::Loop(expr) => depth(expr),
        Expr::Block(exprs) => exprs.iter().map(|expr| depth(expr)).max().unwrap_or(0),
        Expr::Break(expr) => depth(expr),
        Expr::Set(_, expr) => depth(expr),
        Expr::Call(_, exprs) => {
            let mut sum = 0;
            let mut i = 0;
            for elem in exprs.iter(){
                sum = sum.max(depth(elem) + i);
                i += 1;
            }
            sum
        },
        Expr::Tuples(exprs) => {
            let mut sum = 0;
            let mut i = 0;
            for elem in exprs.iter(){
                sum = sum.max(depth(elem) + i);
                i += 1;
            }
            sum
        },
        Expr::Vset(expr1,expr2,expr3) => depth(expr1).max(depth(expr2) + 1).max(depth(expr3) + 2).max(2),
    }
}

fn compile_program(p: &Program) -> (String,String) {
    let mut labels = 0;
    let n_loop = "no_loop".to_string();
    let environment = HashMap::new();
    let mut func_list = HashMap::new();
    let mut defs : String = String::new();
    for def in &p.defs[..] {//Pre insert function name for futher reference
        match def {
            Fun(n, arg, _body) =>{
                if func_list.contains_key(&n.to_string()){
                    panic!("Invalid duplicated function name");
                }
                func_list.insert(n.to_string(),arg.len() as i32);
            }
        }
    }
    for def in &p.defs[..] {
        defs.push_str(&compile_definition(&def, &mut func_list,&mut labels));
    }
    let depth = depth(&p.main);
    
    let offset = if depth % 2 == 0 {depth * 8} else {(depth+1)*8};
    let main = compile(&p.main, 0, &environment, &mut func_list, false, &n_loop, &mut labels);
    let main_with_offsetting = format!("
        sub rsp, {offset}
        {main}
        add rsp, {offset}
    ");
    (defs,main_with_offsetting)
}

fn compile_definition(d: &Definition, func_env:& mut im::HashMap<String,i32> ,labels: &mut i32) -> String {
    match d {
        Fun(name,arg, body) => {
            let mut name_set = HashSet::new();
            for elem in arg{
                if name_set.contains(elem) {
                    panic!("Invalid duplicate parameter name");
                }
                else{
                    name_set.insert(elem);
                }
            }
            let mut body_env = HashMap::new();
            let mut depth = depth(body);
            depth = if depth % 2 ==0 {depth + 1} else {depth};
            let offset = depth * 8;
            let mut index = 1;
            for elem in arg {
                body_env.insert(elem.to_string(), (depth + index) * 8);
                index += 1;
            }
            let n_loop = "no_loop".to_string();
            let body_instrs = compile(body,0, &body_env, func_env,true,&n_loop, labels);
            format!(
                "{name}:
                sub rsp, {offset}
                {body_instrs}
                add rsp, {offset}
                ret\n"
            )
        }
    }
}

fn main() -> std::io::Result<()> {
    let args: Vec<String> = env::args().collect();

    let in_name = &args[1];
    let out_name = &args[2];

    // You will make result hold the result of actually compiling
    let mut in_file = File::open(in_name)?;
    let mut in_contents = String::new();
    in_file.read_to_string(&mut in_contents)?;
    
    //news for program
    let prog = "(".to_owned() + &in_contents + ")";

    let prog_text = parse(&prog);
    if let Err(_e) = prog_text {
        panic!("Invalid");
    }

    let prog = parse_program(&parse(&prog).unwrap());
    let (defs, main) = compile_program(&prog);
    let asm_program = format!(
        "
section .text
global our_code_starts_here
extern snek_print
extern snek_error
extern snek_eq

throw_error:
  
  push rsp
  call snek_error
  ret
{}
our_code_starts_here:
  mov r15, rsi
  {}
  ret
",
        defs,
        main
    );

    let mut out_file = File::create(out_name)?;
    out_file.write_all(asm_program.as_bytes())?;

    Ok(())
}