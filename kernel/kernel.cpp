#include <limine.h>
#include <stdio.h>
#include <arch/arch.hpp>
#include <drivers/graphics/terminal.hpp>
#include <kernel.hpp>
#include <memory/physical.hpp>
#include <memory/virtual.hpp>
#include <sys/panic.hpp>
#include "sys/logger.hpp"

uintptr_t hhdm_offset = 0;

volatile limine_hhdm_request hhdm_request{.id = LIMINE_HHDM_REQUEST,
                                          .revision = 0};

volatile limine_terminal_request terminal_request{.id = LIMINE_TERMINAL_REQUEST,
                                                  .revision = 0};

volatile limine_memmap_request memmap_request{.id = LIMINE_MEMMAP_REQUEST,
                                              .revision = 0};

volatile limine_kernel_file_request kernel_file_request{
    .id = LIMINE_KERNEL_FILE_REQUEST,
    .revision = 0};

volatile limine_kernel_address_request kernel_address_request{
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0};

extern "C" void _start(void) {
    hhdm_offset = hhdm_request.response->offset;

    drivers::display::terminal::init();

    system::gdt::init();
    system::idt::init();

    system::cpu::interrupts_enable();

    memory::physical::init();
    memory::paging::init();

    printf("\n");

    logger::log_info("Total Memory: %i", memory::physical::total());
    logger::log_info("Usable Memory: %i", memory::physical::usable());
    logger::log_info("Used Memory: %i", memory::physical::used());
    logger::log_info("Free Memory: %i", memory::physical::free());

    // panic::panic("Panic test");

    system::cpu::halt(true);
}