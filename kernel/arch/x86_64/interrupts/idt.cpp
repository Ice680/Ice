#include <arch/x86_64/cpu/cpu.hpp>
#include <arch/x86_64/interrupts/idt.hpp>
#include <cstdint>
#include <sys/logger.hpp>

extern "C" void load_idt(uint64_t idt_descriptor);
extern uintptr_t _isr_vector_asm[];

namespace system::idt {
static idt_entry_t idt[256];
static idt_descriptor_t idt_descriptor;

void create_entry(uint8_t index, uint8_t type_attributes) {
    uint64_t offset = _isr_vector_asm[index];  // ISR handler address

    idt[index].offset_low = offset & 0xFFFF;
    idt[index].selector = 0x08;  // kernel code segment
    idt[index].ist = 0;
    idt[index].type_attributes = type_attributes;
    idt[index].offset_middle = (offset >> 16) & 0xFFFF;
    idt[index].offset_high = (offset >> 32) & 0xFFFFFFFF;
    idt[index].zero = 0;
}

void init() {
    logger::log_info("Initializing IDT");

    uint16_t i = 0;

    // exceptions
    for (; i < 32; i++)
        create_entry(i, INT_GATE);

    // standard ISA IRQs
    for (; i < 48; i++)
        create_entry(i, INT_GATE);

    // remaining IRQs
    for (; i < 256; i++)
        create_entry(i, INT_GATE);

    idt_descriptor.limit = sizeof(idt) - 1;
    idt_descriptor.base = reinterpret_cast<uint64_t>(&idt);

    load();

    system::cpu::
        interrupts_enable();  // store interrupt flag -> allow hardware interrupts
}

void load() {
    load_idt(reinterpret_cast<uintptr_t>(&idt));
}
}  // namespace system::idt