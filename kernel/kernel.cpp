#include <drivers/graphics/printf.h>
#include <limine.h>
#include <arch/arch.hpp>
#include <drivers/graphics/terminal.hpp>
#include <kernel.hpp>

volatile limine_terminal_request terminal_request = {
    .id = LIMINE_TERMINAL_REQUEST,
    .revision = 0};

void done() {
    for (;;) {
        asm("hlt");
    }
}

extern "C" void _start(void) {
    drivers::display::terminal::init();

    system::gdt::gdt ggdt;
    system::gdt::gdt_descriptor_t gdt_descriptor{ggdt};
    gdt_descriptor.load();

    drivers::display::terminal::print("\n\t Hello World!");

    done();
}