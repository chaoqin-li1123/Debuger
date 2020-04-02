#ifndef REGISTER_H
#define REGISTER_H
#include <iostream>
#include <string>
#include <vector>
#include<sys/user.h>
using std::string;
using std::vector;
using std::cerr;
using std::cout;
using std::endl;
enum class reg {
    rax, rbx, rcx, rdx,
    rdi, rsi, rbp, rsp,
    r8,  r9,  r10, r11,
    r12, r13, r14, r15,
    rip, rflags,    cs,
    orig_rax, fs_base,
    gs_base,
    fs, gs, ss, ds, es
};

const size_t REG_CNT = 27;

struct reg_descriptor {
	reg_descriptor(reg r_, int dwarf_r_, string reg_name_): 
	r{r_}, dwarf_r{dwarf_r_}, reg_name{reg_name_} {};
    reg r;
    int dwarf_r;
    string reg_name;
};

const reg_descriptor reg_descriptors[REG_CNT]{
    { reg::r15, 15, "r15" },
    { reg::r14, 14, "r14" },
    { reg::r13, 13, "r13" },
    { reg::r12, 12, "r12" },
    { reg::rbp, 6, "rbp" },
    { reg::rbx, 3, "rbx" },
    { reg::r11, 11, "r11" },
    { reg::r10, 10, "r10" },
    { reg::r9, 9, "r9" },
    { reg::r8, 8, "r8" },
    { reg::rax, 0, "rax" },
    { reg::rcx, 2, "rcx" },
    { reg::rdx, 1, "rdx" },
    { reg::rsi, 4, "rsi" },
    { reg::rdi, 5, "rdi" },
    { reg::orig_rax, -1, "orig_rax" },
    { reg::rip, -1, "rip" },
    { reg::cs, 51, "cs" },
    { reg::rflags, 49, "eflags" },
    { reg::rsp, 7, "rsp" },
    { reg::ss, 52, "ss" },
    { reg::fs_base, 58, "fs_base" },
    { reg::gs_base, 59, "gs_base" },
    { reg::ds, 53, "ds" },
    { reg::es, 50, "es" },
    { reg::fs, 54, "fs" },
    { reg::gs, 55, "gs" },
};


uint64_t get_register_value(pid_t pid, reg r) {
    user_regs_struct regs;
    ptrace(PTRACE_GETREGS, pid, nullptr, &regs);
    int idx = 0;
    for (reg_descriptor rd: reg_descriptors) {
    	if (rd.r == r) break;
    	idx++;
    }
    return reinterpret_cast<uint64_t*>(&regs)[idx];
}

void set_register_value(pid_t pid, reg r, uint64_t value) {
    user_regs_struct regs;
    ptrace(PTRACE_GETREGS, pid, nullptr, &regs);
    int idx = 0;
    for (reg_descriptor rd: reg_descriptors) {
    	if (rd.r == r) break;
    	idx++;
    }
    reinterpret_cast<uint64_t*>(&regs)[idx] = value;
    ptrace(PTRACE_GETREGS, pid, nullptr, &regs);
}

uint64_t get_register_value_by_reg_num(pid_t pid, int reg_num) {
    user_regs_struct regs;
    ptrace(PTRACE_GETREGS, pid, nullptr, &regs);
    int idx = 0;
    for (reg_descriptor rd: reg_descriptors) {
    	if (rd.dwarf_r == reg_num) break;
    	idx++;
    }
    //if (idx >= REG_CNT) throw std::out_of_range{"Unknown dwarf register"};
    return reinterpret_cast<uint64_t*>(&regs)[idx];
}

string get_register_name(reg r) {
    for (reg_descriptor rd: reg_descriptors) {
    	if (rd.r == r) return rd.reg_name;
    }	
}

reg get_register_by_name(pid_t pid, const string& reg_name) {
    for (reg_descriptor rd: reg_descriptors) {
    	if (rd.reg_name == reg_name) return rd.r;
    }	
	cerr << "Register " << reg_name << " doesn't exist." << endl;  
}

uint64_t get_register_value_by_reg_name(pid_t pid, const string& reg_name) {
    return get_register_value(pid, get_register_by_name(pid, reg_name));
}


#endif