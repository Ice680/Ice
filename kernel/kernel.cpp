#include <kernel.h>
#include <drivers/graphics/terminal.hpp>
#include <drivers/graphics/printf.h>

volatile limine_terminal_request terminal_request = {
    .id = LIMINE_TERMINAL_REQUEST,
    .revision = 0};

void done() {
    for (;;) {
        asm("hlt");
    }
}

extern "C" void _start(void) {
    using namespace drivers::display;
    
    terminal::init();
    terminal::print("Hello World!\n");
    terminal::cursor_down();
    terminal::printi(123);
    terminal::cursor_right(5);
    terminal::print("Hello World!\n");
    terminal::cursor_up(6);
    terminal::printi(123);
    terminal::cursor_left(10);
    
    done();
}