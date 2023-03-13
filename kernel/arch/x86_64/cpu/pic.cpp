#include <arch/x86_64/cpu/cpu.hpp>
#include <arch/x86_64/cpu/pic.hpp>
#include <cstdint>
#include <sys/logger.hpp>

namespace system::pic {
void eoi(uint64_t int_no) {
    if (int_no >= 40)
        system::cpu::out8(PIC2_COMMAND, PIC_EOI);
    system::cpu::out8(PIC1_COMMAND, PIC_EOI);
}

void mask(uint8_t irq) {
    uint16_t port = 0x21;

    if (irq >= 8) {
        port = 0xA1;
        irq -= 8;
    }

    system::cpu::out8(port, system::cpu::in8(port) | (1 << irq));
}

void unmask(uint8_t irq) {
    uint16_t port = 0x21;

    if (irq >= 8) {
        port = 0xA1;
        irq -= 8;
    }

    system::cpu::out8(port, system::cpu::in8(port) & ~(1 << irq));
}

void disable() {
    system::cpu::out8(0xA1, 0xFF);
    system::cpu::out8(0x21, 0xFF);
}

void init() {
    logger::log_info("Initializing PIC");

    using namespace system::cpu;

    uint8_t a1 = in8(0x21);
    uint8_t a2 = in8(0xA1);

    out8(0x20, 0x11);
    out8(0xA0, 0x11);

    out8(0x21, 0x20);
    out8(0xA1, 0x28);
    out8(0x21, 0x04);
    out8(0xA1, 0x02);
    out8(0x21, 0x01);
    out8(0xA1, 0x01);

    out8(0x21, a1);
    out8(0xA1, a2);
}
}  // namespace system::pic