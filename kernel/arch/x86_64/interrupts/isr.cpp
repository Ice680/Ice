#include <arch/x86_64/cpu/cpu.hpp>
#include <arch/x86_64/interrupts/isr.hpp>
#include <sys/logger.hpp>

namespace system::isr {
static const char* exceptions[] = {
    "#DE: Divide Error",
    "#DB: logger::log_error Exception",
    " — : NMI Interrupt",
    "#BP: Breakpoint",
    "#OF: Overflow",
    "#BR: BOUND Range Exceeded",
    "#UD: Invalid Opcode (Undefined Opcode)",
    "#NM: Device Not Available (No Math Coprocessor)",
    "#DF: Double Fault",
    "— : Coprocessor Segment Overrun (reserved)",
    "#TS: Invalid TSS",
    "#NP: Segment Not Present",
    "#SS: Stack-Segment Fault",
    "#GP: General Protection",
    "#PF: Page Fault",
    "— : (Intel reserved. Do not use.)",
    "#MF: x87 FPU Floating-Point Error (Math Fault)",
    "#AC: Alignment Check",
    "#MC: Machine Check",
    "#XM: SIMD Floating-Point Exception",
    "#VE: Virtualization Exception",
    "#CP: Control Protection Exception",
    "— : Intel reserved. Do not use.",
    "— : Intel reserved. Do not use.",
    "— : Intel reserved. Do not use.",
    "— : Intel reserved. Do not use.",
    "— : Intel reserved. Do not use.",
    "— : Intel reserved. Do not use.",
    "— : Intel reserved. Do not use.",
    "— : Intel reserved. Do not use.",
    "— : Intel reserved. Do not use.",
    "— : Intel reserved. Do not use."};

static const char* isa_irqs[] = {"Programmable Interrupt Timer",
                                 "Keyboard",
                                 "Cascade",
                                 "COM2",
                                 "COM1",
                                 "LPT2",
                                 "Floppy Disk",
                                 "LPT1 / Unreliable spurious interrupt",
                                 "CMOS real-time clock",
                                 "Free for peripherals / legacy SCSI / NIC",
                                 "Free for peripherals / SCSI / NIC",
                                 "Free for peripherals / SCSI / NIC",
                                 "PS2 Mouse",
                                 "FPU / Coprocessor / Inter-processor",
                                 "Primary ATA Hard Disk",
                                 "Secondary ATA Hard Disk"};

extern "C" uint64_t isr_handler(uint64_t rsp) {
    system::cpu::cpu_interrupt_state_t* cpu =
        reinterpret_cast<system::cpu::cpu_interrupt_state_t*>(rsp);

    // handle exceptions
    if (cpu->isr_number < 32) {
        logger::log_error("\n────────────────────────\n");
        logger::log_error("⚠ EXCEPTION OCCURRED! ⚠\n\n");
        logger::log_error("⤷ ISR-No. %d: %s\n", cpu->isr_number,
                          exceptions[cpu->isr_number]);
        logger::log_error("⤷ Error code: 0x%.16llx\n\n\n", cpu->error_code);
        isr_register_dump(cpu);

        for (;;) {
            system::cpu::halt();
        }
    } else if (cpu->isr_number >= 32 &&
               cpu->isr_number < 48) {  // handle ISA IRQs
        if (cpu->isr_number == LAPIC_TIMER_INT) {
            logger::log_info(".");
        } else {
            logger::log_error("\n────────────────────────\n");
            logger::log_error("⚠ UNHANDLED HARDWARE INTERRUPT OCCURRED! ⚠\n\n");
            logger::log_error("⤷ ISR-No. %d: %s\n", cpu->isr_number,
                              isa_irqs[cpu->isr_number - 32]);
            isr_register_dump(cpu);
        }

        for (;;) {
            system::cpu::halt();
        }
    } else if (cpu->isr_number == SYSCALL_INT) {  // handle syscalls
        // TODO syscalls

        logger::log_error("\n────────────────────────\n");
        logger::log_error("⚠ SYSCALL OCCURRED - UNHANDLED FOR NOW! ⚠\n\n");
        isr_register_dump(cpu);

        for (;;) {
            system::cpu::halt();
        }
    } else {  // handle unknown interrupts
        logger::log_error("\n────────────────────────\n");
        logger::log_error("⚠ UNKNOWN INTERRUPT OCCURRED! ⚠\n\n");
        logger::log_error("⤷ ISR-No. %d\n", cpu->isr_number);
        isr_register_dump(cpu);

        for (;;) {
            system::cpu::halt();
        }
    }

    return rsp;
}

void isr_register_dump(system::cpu::cpu_interrupt_state_t* cpu) {
    logger::log_error("ℹ Register dump:\n\n");
    logger::log_error(
        "⤷ rax: 0x%.16llx, rbx:    0x%.16llx, rcx: 0x%.16llx, rdx: 0x%.16llx\n"
        "⤷ rsi: 0x%.16llx, rdi:    0x%.16llx, rbp: 0x%.16llx, r8 : 0x%.16llx\n"
        "⤷ r9 : 0x%.16llx, r10:    0x%.16llx, r11: 0x%.16llx, r12: 0x%.16llx\n"
        "⤷ r13: 0x%.16llx, r14:    0x%.16llx, r15: 0x%.16llx, ss : 0x%.16llx\n"
        "⤷ rsp: 0x%.16llx, rflags: 0x%.16llx, cs : 0x%.16llx, rip: 0x%.16llx\n",
        cpu->rax, cpu->rbx, cpu->rcx, cpu->rdx, cpu->rsi, cpu->rdi, cpu->rbp,
        cpu->r8, cpu->r9, cpu->r10, cpu->r11, cpu->r12, cpu->r13, cpu->r14,
        cpu->r15, cpu->ss, cpu->rsp, cpu->rflags, cpu->cs, cpu->rip);
}
}  // namespace system::isr