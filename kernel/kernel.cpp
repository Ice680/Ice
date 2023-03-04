#include <kernel.h>
#include <drivers/graphics/terminal.hpp>
#include <drivers/graphics/printf.h>
#include <arch/arch.hpp>
#include "arch/x86_64/gdt/gdt.hpp"

#define V2P(a) ((uint64_t)(a)-(uint64_t)0xffffffff80000000)

volatile limine_terminal_request terminal_request = {
    .id = LIMINE_TERMINAL_REQUEST,
    .revision = 0};

void done() {
    for (;;) {
        asm("hlt");
    }
}

extern "C" void _start(void) {
    using namespace drivers;
    
    display::terminal::init();

    system::gdt::gdt ggdt;
    system::gdt::gdt_descriptor_t gdt_descriptor{ggdt};
    gdt_descriptor.load();
    
    display::terminal::print("\n\t Hello World!");
    
    done();
}