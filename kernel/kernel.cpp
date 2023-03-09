#include <limine.h>
#include <stdio.h>
#include <arch/arch.hpp>
#include <drivers/graphics/terminal.hpp>
#include <kernel.hpp>
#include <memory/physical.hpp>
#include <sys/panic.hpp>

uintptr_t hhdm_offset = 0;

volatile limine_hhdm_request hhdm_request{.id = LIMINE_HHDM_REQUEST,
                                          .revision = 0};

volatile limine_terminal_request terminal_request{.id = LIMINE_TERMINAL_REQUEST,
                                                  .revision = 0};

volatile limine_memmap_request memmap_request{.id = LIMINE_MEMMAP_REQUEST,
                                              .revision = 0};

extern "C" void _start(void) {
    hhdm_offset = hhdm_request.response->offset;

    drivers::display::terminal::init();

    system::gdt::init();
    system::idt::init();

    system::cpu::interrupts_enable();

    memory::physical::init();

    printf("\nHello World!");
    putchar('\n');

    // panic::panic("Panic test");

    system::cpu::halt(true);
}