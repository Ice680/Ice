#pragma once

#include <drivers/graphics/printf.h>
#include <limine.h>
#include <stdarg.h>
#include <stdint.h>
#include <common/math.hpp>

namespace drivers::display::terminal {
extern limine_terminal** terminals;
extern limine_terminal* main_term;
extern uint64_t term_count;

void print(const char* string, limine_terminal* term = main_term);
void printi(int i, limine_terminal* term = main_term);
void printc(char c, limine_terminal* term = main_term);

void init();

void reset(limine_terminal* term = main_term);
void clear(const char* ansii_color = "\033[0m",
           limine_terminal* term = main_term);
void center(const char* text, limine_terminal* term = main_term);

void cursor_up(int lines = 1, limine_terminal* term = main_term);
void cursor_down(int lines = 1, limine_terminal* term = main_term);
void cursor_right(int lines = 1, limine_terminal* term = main_term);
void cursor_left(int lines = 1, limine_terminal* term = main_term);
}  // namespace drivers::display::terminal

static inline int vprintf(limine_terminal* term, const char* format,
                          va_list arg) {
    auto printc = reinterpret_cast<void (*)(char, void*)>(
        drivers::display::terminal::printc);
    int ret = vfctprintf(printc, term, format, arg);
    return ret;
}

static inline int printf(limine_terminal* term, const char* format, ...) {
    va_list arg;

    va_start(arg, format);
    int ret = vprintf(term, format, arg);
    va_end(arg);

    return ret;
}