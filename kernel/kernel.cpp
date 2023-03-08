#include <limine.h>
#include <stdio.h>
#include <arch/arch.hpp>
#include <drivers/graphics/terminal.hpp>
#include <kernel.hpp>
#include <sys/panic.hpp>

volatile limine_terminal_request terminal_request = {
    .id = LIMINE_TERMINAL_REQUEST,
    .revision = 0};

extern "C" void _start(void) {
    drivers::display::terminal::init();

    system::gdt::init();
    system::idt::init();

    system::cpu::interrupts_enable();

    printf("\nHello World!");
    putchar('\n');

    // panic::panic("Panic test");

    system::cpu::halt(true);
}