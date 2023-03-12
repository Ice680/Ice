#pragma once

#if defined(x86_64)
#include <arch/x86_64/cpu/cpu.hpp>
#include <arch/x86_64/gdt/gdt.hpp>
#include <arch/x86_64/interrupts/idt.hpp>
#include <arch/x86_64/interrupts/isr.hpp>
#endif