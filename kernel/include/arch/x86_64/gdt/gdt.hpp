#pragma once
#include <stddef.h>
#include <stdint.h>

// assembly functions
extern "C" void load_gdt(void const* ptr);

namespace system::gdt {
struct [[gnu::packed]] gdt_entry_t {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t flags;
    uint8_t limit_high : 4;
    uint8_t granularity : 4;
    uint8_t base_high;

    constexpr gdt_entry_t() = default;

    constexpr gdt_entry_t(uint8_t _flags, uint8_t _granularity)
        : flags(_flags), granularity(_granularity){};

    constexpr gdt_entry_t(uint32_t _base, uint32_t _limit, uint8_t _flags,
                          uint8_t _granularity)
        : limit_low(_limit & 0xFFFF),
          base_low(_base & 0xFFFF),
          base_middle((_base >> 16) & 0xFF),
          flags(_flags),
          limit_high((_limit >> 16) & 0x0F),
          granularity(_granularity),
          base_high((_base >> 24) & 0xFF) {}
};

struct [[gnu::packed]] gdt {
    gdt();

    enum selector {
        ZERO = 0,
        KERNEL_CODE = 1,
        KERNEL_DATA = 2,
        USER_DATA = 3,
        USER_CODE = 4,
        TSS = 5
    };
    enum flags : uint32_t {
        SEGMENT = 0b00010000,
        PRESENT = 0b10000000,
        USER = 0b01100000,
        EXECUTABLE = 0b00001000,
        READ_WRITE = 0b00000010
    };
    enum granularity { LONG_MODE = 0b10 };
    gdt_entry_t entries[5];
};

struct [[gnu::packed]] gdt_descriptor_t {
    uint16_t limit;
    uint64_t base;

    gdt_descriptor_t(gdt const& base)
        : limit(sizeof(gdt) - 1), base(reinterpret_cast<size_t>(&base)) {}

    void load() const;
};
}  // namespace system::gdt