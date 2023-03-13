#pragma once

#include <arch/x86_64/cpu/cpu.hpp>
#include <cstddef>
#include <cstdint>

namespace system::idt {
enum IRQ {
    IRQ0 = 32,
    IRQ1 = 33,
    IRQ2 = 34,
    IRQ3 = 35,
    IRQ4 = 36,
    IRQ5 = 37,
    IRQ6 = 38,
    IRQ7 = 39,
    IRQ8 = 40,
    IRQ9 = 41,
    IRQ10 = 42,
    IRQ11 = 43,
    IRQ12 = 44,
    IRQ13 = 45,
    IRQ14 = 46,
    IRQ15 = 47
};

struct [[gnu::packed]] idt_entry_t {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attributes;
    uint8_t offset_middle;
    uint32_t offset_high;
    uint32_t zero;
};

struct [[gnu::packed]] idt_descriptor_t {
    uint16_t limit;
    uint64_t base;
};

struct interrupt_handler_t {
    uintptr_t handler;
    uint64_t args;
    bool arg = false;
};

using interrupt_handler_func = void (*)(system::cpu::cpu_interrupt_state_t*);
using interrupt_handler_func_arg = void (*)(system::cpu::cpu_interrupt_state_t*, uint64_t);

extern idt_entry_t idt[];
extern idt_descriptor_t idt_descriptor;
extern interrupt_handler_t interrupt_handlers[];
extern "C" void* interrupt_table[];

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t type_attributes = 0x8E, uint8_t ist = 0);

uint8_t alloc_vector();

void register_interrupt_handler(uint8_t vector, interrupt_handler_func handler);
void register_interrupt_handler(uint8_t vector, interrupt_handler_func_arg handler, uint64_t args);

void reload();
void init();
}  // namespace system::idt