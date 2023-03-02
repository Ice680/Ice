#pragma once

#include <drivers/graphics/printf.h>
#include <limine.h>
#include <stdarg.h>
#include <stdint.h>

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
}  // namespace drivers::display::terminal