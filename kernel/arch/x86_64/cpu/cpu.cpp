#include <arch/x86_64/cpu/cpu.hpp>

namespace system::cpu {
void interrupts_enable() {
    asm volatile("sti");
}

void interrupts_disable() {
    asm volatile("cli");
}

void halt() {
    asm volatile("hlt");
}
}  // namespace system::cpu