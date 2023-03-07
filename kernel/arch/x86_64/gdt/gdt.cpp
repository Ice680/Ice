#include <arch/arch.hpp>
#include <drivers/graphics/terminal.hpp>

namespace system::gdt {
void gdt_descriptor_t::load() const {
    load_gdt(this);
}

gdt::gdt() {
    entries[0] = gdt_entry_t();
    entries[1] = {PRESENT | SEGMENT | READ_WRITE | EXECUTABLE, LONG_MODE};
    entries[2] = {PRESENT | SEGMENT | READ_WRITE, 0};
    entries[3] = {PRESENT | SEGMENT | READ_WRITE | USER, 0};
    entries[4] = {PRESENT | SEGMENT | READ_WRITE | EXECUTABLE | USER,
                  LONG_MODE};
}
}  // namespace system::gdt