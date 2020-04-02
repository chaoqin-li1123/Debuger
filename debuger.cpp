#include <iostream>
#include <unistd.h> 
#include <assert.h>
#include <sys/ptrace.h>
#include "debuger.h"
using std::cout;
using std::endl; 


void execute_debugee (const char* prog) {
    if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) {
        std::cerr << "Error in ptrace\n";
        return;
    }
    execl(prog, prog, nullptr);
}

int main(int argc, char* argv[]) {
	assert(argc > 1);
	const char* prog = argv[1];
	auto pid = fork();
	if (pid == 0) {
		execute_debugee(prog);
	}
	else {
		debuger dbg(prog, pid);
		dbg.run();
	}
}