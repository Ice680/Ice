#pragma once

#include <cstdint>

#define wrreg(reg, val) asm volatile("mov %0, %%" #reg ::"r"(val) : "memory");

#define rdreg(reg)                                                \
    ({                                                            \
        uintptr_t val;                                            \
        asm volatile("mov %%" #reg ", %0" : "=r"(val)::"memory"); \
        val;                                                      \
    })

namespace system::cpu {
void interrupts_enable();
void interrupts_disable();
[[noreturn]] void halt(bool ints = false);

void invlpg(uint64_t addr);

bool id(uint32_t leaf, uint32_t subleaf, uint32_t& eax, uint32_t& ebx,
        uint32_t& ecx, uint32_t& edx);

struct cpu_interrupt_state_t {
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;

    uint64_t isr_number;
    uint64_t error_code;

    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
};
}  // namespace system::cpu