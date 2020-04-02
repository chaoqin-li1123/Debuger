#ifndef BREAKPOINT_H
#define BREAKPOINT_H
#include <sys/ptrace.h>
const uint64_t byte_mask = 0xff;
// Breakpoint class
class breakpoint{
public:
	breakpoint(pid_t pid_, intptr_t addr_): pid{pid_}, addr{addr_}{}
	void enable() {
		auto data = ptrace(PTRACE_PEEKDATA, pid, addr, nullptr);
		saved_data = static_cast<uint8_t>(data & byte_mask); 
		uint64_t int3 = 0xcc;
		uint64_t data_with_int3 = ((data & ~byte_mask) | int3);
		ptrace(PTRACE_POKEDATA, pid, addr, data_with_int3);
		enabled = true;
	}
	void disable() {
		auto data = ptrace(PTRACE_PEEKDATA, pid, addr, nullptr);
		uint64_t original_data = ((data & ~byte_mask) | saved_data);
		ptrace(PTRACE_POKEDATA, pid, addr, original_data);
		enabled = false;
	}
	bool is_enabled() const {
		return enabled;
	}
	bool get_address() const {
		return addr;
	}
private:
	pid_t pid;
	intptr_t addr;
	bool enabled = false;
	uint8_t saved_data;
};

#endif