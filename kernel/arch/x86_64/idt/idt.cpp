#include <arch/x86_64/cpu/cpu.hpp>
#include <arch/x86_64/cpu/pic.hpp>
#include <arch/x86_64/idt/idt.hpp>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <sys/logger.hpp>
#include "memory/virtual.hpp"
#include "sys/panic.hpp"

namespace system::idt {
idt_entry_t idt[256];
idt_descriptor_t idt_descriptor;
interrupt_handler_t interrupt_handlers[256];

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t type_attributes,
                        uint8_t ist) {
    idt[vector].offset_low = reinterpret_cast<uint64_t>(isr);
    idt[vector].selector = 0x08;
    idt[vector].ist = ist;
    idt[vector].type_attributes = type_attributes;
    idt[vector].offset_middle = reinterpret_cast<uint64_t>(isr) >> 16;
    idt[vector].offset_high = reinterpret_cast<uint64_t>(isr) >> 32;
    idt[vector].zero = 0;
}

void reload() {
    asm volatile("cli");
    asm volatile("lidt %0" : : "memory"(idt_descriptor));
    asm volatile("sti");
}

void init() {
    logger::log_info("Initializing IDT");

    idt_descriptor.limit = sizeof(idt_entry_t) * 256 - 1;
    idt_descriptor.base = reinterpret_cast<uintptr_t>(&idt[0]);

    for(size_t i = 0; i < 256; i++) idt_set_descriptor(i, interrupt_table[i]);

    system::pic::init();

    reload();
}

static uint8_t next_free = 48;

uint8_t alloc_vector() {
    return next_free;
}

void register_interrupt_handler(uint8_t vector,
                                interrupt_handler_func handler) {
    interrupt_handlers[vector].handler = reinterpret_cast<uint64_t>(handler);
    interrupt_handlers[vector].arg = false;
}

void register_interrupt_handler(uint8_t vector,
                                interrupt_handler_func_arg handler,
                                uint64_t args) {
    interrupt_handlers[vector].handler = reinterpret_cast<uint64_t>(handler);
    interrupt_handlers[vector].arg = true;
    interrupt_handlers[vector].args = args;
}

static const char* exception_messages[32] = {
    "Division by zero",
    "Debug",
    "Non-maskable interrupt",
    "Breakpoint",
    "Detected overflow",
    "Out-of-bounds",
    "Invalid opcode",
    "No coprocessor",
    "Double fault",
    "Coprocessor segment overrun",
    "Bad TSS",
    "Segment not present",
    "Stack fault",
    "General protection fault",
    "Page fault",
    "Unknown interrupt",
    "Coprocessor fault",
    "Alignment check",
    "Machine check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
};

// static volatile bool halt = true;

static void exception_handler(system::cpu::cpu_interrupt_state_t* regs) {
    logger::log_error("System exception");
    logger::log_error("Exception: %s", exception_messages[regs->int_no]);
    logger::log_error("Address: 0x1X", regs->rip);
    logger::log_error("Error Code: 0x%1X, 0b%b", regs->error_code,
                      regs->error_code);

    printf("\n[\033[31mPANIC\033[0m] System Exception!\n");
    printf("[\033[31mPANIC\033[0m] Exception: %s\n",
           exception_messages[regs->int_no]);
    printf("[\033[31mPANIC\033[0m] Address: 0x%lX\n", regs->rip);

    switch (regs->int_no) {
        case 8:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
            printf("[\033[31mPANIC\033[0m] Error code: 0x%lX\n",
                   regs->error_code);
            break;
    }

    printf("[\033[31mPANIC\033[0m] System halted!\n");
    logger::log_error("System halted!\n");

    system::cpu::halt();
}

static void irq_handler(system::cpu::cpu_interrupt_state_t* regs) {
    if (interrupt_handlers[regs->int_no].handler) {
        if (interrupt_handlers[regs->int_no].arg) {
            reinterpret_cast<interrupt_handler_func_arg>(
                interrupt_handlers[regs->int_no].handler)(
                regs, interrupt_handlers[regs->int_no].args);
        } else {
            reinterpret_cast<interrupt_handler_func>(
                interrupt_handlers[regs->int_no].handler)(regs);
        }
    }

    system::pic::eoi(regs->int_no);
}

extern "C" void interrupt_handler(system::cpu::cpu_interrupt_state_t* regs) {
    if(regs->int_no < 32) exception_handler(regs);
    else if(regs->int_no >= 32 && regs->int_no < 256) irq_handler(regs);
    else panic::panic("Unknown interrupt");
}
}  // namespace system::idt