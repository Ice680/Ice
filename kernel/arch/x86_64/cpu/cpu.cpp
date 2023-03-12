#include <arch/x86_64/cpu/cpu.hpp>
#include <cstdint>

namespace system::cpu {
void interrupts_enable() {
    asm volatile("sti");
}

void interrupts_disable() {
    asm volatile("cli");
}

void halt(bool ints) {
    if (ints)
        while (true)
            asm volatile("hlt");
    else
        while (true)
            asm volatile("cli; hlt");
}

void invlpg(uint64_t addr) {
    asm volatile("invlpg (%0)" ::"r"(addr));
}

bool id(uint32_t leaf, uint32_t subleaf, uint32_t& eax, uint32_t& ebx,
        uint32_t& ecx, uint32_t& edx) {
    uint32_t cpuid_max = 0;

    asm volatile("cpuid"
                 : "=a"(cpuid_max)
                 : "a"(leaf & 0x80000000)
                 : "ebx", "ecx", "edx");

    if (leaf > cpuid_max)
        return false;

    asm volatile("cpuid"
                 : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                 : "a"(leaf), "c"(subleaf));

    return true;
}
}  // namespace system::cpu