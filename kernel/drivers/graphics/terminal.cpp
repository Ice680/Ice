#include <drivers/graphics/printf.h>
#include <kernel.h>
#include <limine.h>
#include <drivers/graphics/terminal.hpp>

size_t strlen(const char* str) {
    if (str == nullptr)
        return 0;
    size_t length = 0;
    while (str[length])
        length++;
    return length;
}

namespace drivers::display::terminal {
limine_terminal** terminals;
limine_terminal* main_term;
uint64_t term_count;

void init() {
    terminals = terminal_request.response->terminals;
    main_term = terminals[0];
    term_count = terminal_request.response->terminal_count;
}

void print(const char* str, limine_terminal* term) {
    if (terminal_request.response == nullptr || term == nullptr)
        return;
    terminal_request.response->write(term, str, strlen(str));
}

void printi(int num, limine_terminal* term) {
    if (num != 0) {
        char temp[10];
        int i = 0;

        if (num < 0) {
            printc('-');
            num = -num;
        }

        if (num <= 0) {
            temp[i++] = '8';
            num = -(num / 10);
        }
        while (num > 0) {
            temp[i++] = num % 10 + '0';
            num /= 10;
        }

        while (--i >= 0)
            printc(temp[i], term);
    } else {
        printc('0', term);
    }
}

void printc(char c, limine_terminal* term) {
    char str[] = {c, 0};
    print(str, term);
}

void reset(limine_terminal* term) {
    if (terminal_request.response == nullptr || term == nullptr)
        return;
    terminal_request.response->write(term, "", LIMINE_TERMINAL_FULL_REFRESH);
}

void clear(const char* ansii_color, limine_terminal* term) {
    print(ansii_color, term);
    print("\033[H\033[2J", term);
}

void center(const char* text, limine_terminal* term) {
    for (uint64_t i = 0; i < term->columns / 2 - strlen(text) / 2; i++)
        printc(' ');
    print(text);
    for (uint64_t i = 0; i < term->columns / 2 - strlen(text) / 2; i++)
        printc(' ');
}

void cursor_up(int lines, limine_terminal* term) {
    printf(term, "\033[dA", lines);
}

void cursor_down(int lines, limine_terminal* term) {
    printf(term, "\033[%dB", lines);
}

void cursor_right(int lines, limine_terminal* term) {
    printf(term, "\033[%dC", lines);
}

void cursor_left(int lines, limine_terminal* term) {
    printf(term, "\033[%dD", lines);
}
}  // namespace drivers::display::terminal

void putchar(char c) {
    drivers::display::terminal::printc(c);
}