#pragma once

#include <cstddef>
#include <cstdint>

#define INT_GATE 0x8E

namespace system::idt {
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

void create_entry(uint8_t index, uint8_t type_attributes);
void init();
void load();
}  // namespace system::idt