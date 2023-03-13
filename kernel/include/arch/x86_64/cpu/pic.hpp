#pragma once

#include <cstdint>

namespace system::pic {
enum {
    PIC1 = 0x20,
    PIC2 = 0xA0,
    PIC1_COMMAND = PIC1,
    PIC1_DATA = 0x21,
    PIC2_COMMAND = PIC1,
    PIC2_DATA = 0xA1,
    PIC_EOI = 0x20
};

void eoi(uint64_t int_no);
void mask(uint8_t irq);
void unmask(uint8_t irq);

void disable();
void init();
}