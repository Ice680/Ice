#pragma once

#include <stdint.h>
#include <arch/x86_64/cpu/cpu.hpp>

#define LAPIC_TIMER_INT 32
#define SYSCALL_INT 128
#define SPURIOUS_INT 255

namespace system::isr {
extern "C" uint64_t isr_handler(uint64_t rsp);
void isr_register_dump(system::cpu::cpu_interrupt_state_t* cpu);
}  // namespace system::isr