#include <limine.h>
#include <arch/arch.hpp>
#include <drivers/graphics/terminal.hpp>
#include <kernel.hpp>
#include <stdio.h>

volatile limine_terminal_request terminal_request = {
    .id = LIMINE_TERMINAL_REQUEST,
    .revision = 0};

void done() {
    for (;;) {
        system::cpu::halt();
    }
}

extern "C" void _start(void) {
    drivers::display::terminal::init();

    system::gdt::init();
    system::idt::init();

    system::cpu::interrupts_enable();

    printf("\nHello World!");
    putchar('\n');

    done();
}