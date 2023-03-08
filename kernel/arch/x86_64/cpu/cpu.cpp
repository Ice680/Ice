#include <arch/x86_64/cpu/cpu.hpp>

namespace system::cpu {
void interrupts_enable() {
    asm volatile("sti");
}

void interrupts_disable() {
    asm volatile("cli");
}

void halt(bool ints) {
    if(ints) while(true) asm volatile("hlt");
    else while(true) asm volatile("cli; hlt");
}
}  // namespace system::cpu