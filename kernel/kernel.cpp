#include <kernel.h>
#include <drivers/graphics/terminal.hpp>
#include <drivers/graphics/printf.h>

volatile limine_terminal_request terminal_request = {
    .id = LIMINE_TERMINAL_REQUEST,
    .revision = 0};

void done(void) {
    for (;;) {
        asm("hlt");
    }
}

extern "C" void _start(void) {
    using namespace drivers::display;
    
    terminal::init();
    terminal::print("Hello World!");
    terminal::printi(123);
    terminal::clear("\033[2m");
    terminal::reset();
    terminal::print("Hello World!");
    terminal::printi(123);

    done();
}