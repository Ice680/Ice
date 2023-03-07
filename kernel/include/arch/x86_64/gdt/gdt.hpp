#pragma once

#include <stddef.h>
#include <stdint.h>

#define GDT_ENTRIES 6

namespace system::gdt {
enum {
    GDT_LONG_MODE_GRANULARITY = 0x20,
    GDT_SEGMENT = 0x10,
    GDT_PRESENT = 0x80,
    GDT_TSS_PRESENT = 0x80,
    GDT_USER = 0x60,
    GDT_EXECUTABLE = 0x8,
    GDT_READ_WRITE = 0x2,
    GDT_FLAGS = 0xC
};

struct [[gnu::packed]] gdt_entry_t {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
};

struct [[gnu::packed]] gdt_descriptor_t {
    uint16_t segment;
    uint64_t offset;
};

struct gdt {
    gdt_entry_t gdt[GDT_ENTRIES]{};
};


gdt_entry_t create_entry(uint32_t base, uint32_t limit, uint8_t granularity, uint8_t access);
void init();
void load(gdt_descriptor_t* descriptor);
}  // namespace system::gdt