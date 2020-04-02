#ifndef DEBUGER_H
#define DEBUGER_H
#include <string>
#include <iostream>
#include <iomanip>  
#include <vector>
#include <sstream>
#include <unordered_map>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <stdlib.h>
#include <memory>
#include "breakpoint.h"
#include "register.h"
using std::cin;
using std::cout;
using std::endl;
using std::cerr;
using std::string;
using std::getline;
using std::vector;
using std::stringstream;
using std::unordered_map;
using std::make_unique;
using std::unique_ptr;
void input(string hint, string& line) {
	cout << hint;
	getline(cin, line);
}

vector<string> split(const string& str, char delimeter) {
	string token;
	vector<string> tokens;
    stringstream ss(str);
    while (getline(ss, token, delimeter)) {
        tokens.push_back(token);
    }
	return tokens;
}

void print_hex_format(uint64_t x, bool start_new_line) {
	cout << "0x" << std::setfill('0') << std::setw(16) << std::hex 
	     << x;
	if (start_new_line) cout << endl;
}
// Debuger class
class debuger{
public:
	debuger(string debugee_, pid_t pid_): debugee{debugee_}, pid{pid_} {}
	void run() {
		int wait_status;
		auto options = 0;
		waitpid(pid, &wait_status, options);
		while (true) {
			string command;
			input("chaoqin_dbg> ", command); 
			handle_command(command);
		}

	}
	void set_breakpoint(intptr_t addr) {
		cout << "Set breakpoint at address "; 
		print_hex_format(addr, true);
		unique_ptr<breakpoint> bp = make_unique<breakpoint>(pid, addr);
		bp->enable();
		break_table[addr] = move(bp);
	}
	uint64_t read_memory(uint64_t addr) {
		return ptrace(PTRACE_PEEKDATA, pid, addr, nullptr);
	}
	void write_memory(uint64_t addr, uint64_t data) {
		ptrace(PTRACE_POKEDATA, pid, addr, data);
	}
private:
	string debugee;
	pid_t pid;
	unordered_map<intptr_t, unique_ptr<breakpoint>> break_table;
	void handle_command(const string& command) {
		vector<string> args = split(command, ' ');
		string& operation = args[0];
		if (operation == "continue") continue_execution();
		else if (operation == "break") {
			string addr_str = args[1].substr(2);
			set_breakpoint(stol(addr_str, 0, 16));
		}
		else if (operation == "q") {
			exit(EXIT_SUCCESS);
		}
		else if (operation == "register") {
			if (args[1] == "dump") {
				dump_registers();
			}
			else if (args[1] == "read") {
				uint64_t data = get_register_value_by_reg_name(pid, args[2]);
				cout << args[2] << ": ";
				print_hex_format(data, true);
			}
			else if (args[1] == "write") {
				uint64_t data = stol(args[2].substr(2), 0, 16);
				set_register_value(pid, get_register_by_name(pid, args[1]), data);
			}
		}
		else if (operation == "memory") {
			if (args[1] == "read") {
				uint64_t addr = stol(args[1].substr(2), 0, 16);
				uint64_t data = read_memory(addr);
				print_hex_format(addr, false);
				cout << ": ";
				print_hex_format(data, true);
			}
			else if (args[1] == "write") {
				uint64_t addr = stol(args[1].substr(2), 0, 16);
				uint64_t data = stol(args[2].substr(2), 0, 16);
				write_memory(addr, data);
			}
		}
		else cerr << "Unknown command." << endl;
	}
	void dump_registers() {
	    for (reg_descriptor rd: reg_descriptors) {
	    	cout << rd.reg_name << ": ";
	    	print_hex_format(get_register_value(pid, rd.r), true);
	    }	
	}
	void continue_execution() {
		step_over_break_point();
		ptrace(PTRACE_CONT, pid, nullptr, nullptr);
		wait_for_signal();
	}
	void step_over_break_point() {
		uint64_t possible_break_point = get_pc() - 1;
		if (break_table.find(possible_break_point) == break_table.end()) return;
		unique_ptr<breakpoint>& bp = break_table[possible_break_point];
		if (bp->is_enabled()) {
			bp->disable();
			set_pc(possible_break_point);
            ptrace(PTRACE_SINGLESTEP, pid, nullptr, nullptr);
			wait_for_signal();
			bp->enable();
		}
	}
	uint64_t get_pc() {
    	return get_register_value(pid, reg::rip);
	}

	void set_pc(uint64_t pc) {
	    set_register_value(pid, reg::rip, pc);
	}
	void wait_for_signal() {
		int wait_status;
		auto options = 0;
		waitpid(pid, &wait_status, options);		
	}
};


#endif