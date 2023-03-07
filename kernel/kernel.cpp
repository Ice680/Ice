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

    system::gdt::init();
    drivers::display::terminal::print("Loaded GDT\n");
    
    drivers::display::terminal::print("\nHello World!");

    done();
}