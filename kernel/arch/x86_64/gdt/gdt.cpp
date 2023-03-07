#include <arch/x86_64/gdt/gdt.hpp>
#include <cstdint>

#define KERNEL_CODE_SELECTOR 1
#define KERNEL_DATA_SELECTOR 2
#define USER_CODE_SELECTOR 4
#define USER_DATA_SELECTOR 5

namespace system::gdt {
static gdt _gdt;

gdt_descriptor_t _gdt_descriptor = {
    .segment = sizeof(gdt) - 1,
    .offset = reinterpret_cast<uint64_t>(&_gdt)};

void init() {
    _gdt.gdt[0] = create_entry(0, 0, 0, 0);

    _gdt.gdt[KERNEL_CODE_SELECTOR] = create_entry(
        0, 0, GDT_LONG_MODE_GRANULARITY,
        GDT_PRESENT | GDT_SEGMENT | GDT_READ_WRITE | GDT_EXECUTABLE);
    _gdt.gdt[KERNEL_DATA_SELECTOR] =
        create_entry(0, 0, 0, GDT_PRESENT | GDT_SEGMENT | GDT_READ_WRITE);

    _gdt.gdt[3] = create_entry(0, 0, 0, 0);

    _gdt.gdt[USER_CODE_SELECTOR] =
        create_entry(0, 0, GDT_LONG_MODE_GRANULARITY,
                     GDT_PRESENT | GDT_SEGMENT | GDT_READ_WRITE | GDT_USER);
    _gdt.gdt[USER_DATA_SELECTOR] = create_entry(
        0, 0, 0, GDT_PRESENT | GDT_SEGMENT | GDT_READ_WRITE | GDT_USER);

    load(&_gdt_descriptor);
}

gdt_entry_t create_entry(uint32_t base, uint32_t limit, uint8_t granularity,
                         uint8_t access) {
    gdt_entry_t entry;

    entry.base_low = (base & 0xFFFF);
    entry.base_mid = (base >> 16) & 0xFF;
    entry.base_high = (base >> 24) & 0xFF;

    entry.limit_low = (limit & 0xFFFF);
    entry.granularity = (limit >> 16) & 0x0F;
    entry.granularity |= granularity & 0xF0;

    entry.access = access;

    return entry;
}

extern "C" void gdt64_load(gdt_descriptor_t* descriptor, uint64_t data,
                           uint64_t code);

// C++ wrapper
void load(gdt_descriptor_t* descriptor) {
    gdt64_load(descriptor, KERNEL_DATA_SELECTOR * 8, KERNEL_CODE_SELECTOR * 8);
}
}  // namespace system::gdt