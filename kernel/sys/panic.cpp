#include <sys/logger.hpp>
#include <sys/panic.hpp>
#include <source_location>
#include "arch/x86_64/cpu/cpu.hpp"

[[noreturn]] void panic(const char* error, void* stacktrace_addr, const std::source_location location) {
    system::cpu::interrupts_disable();
    
    logger::log_error("Kernel panic - %s", error);
    logger::log_error("Panicked in file %s:%d:%d in the function '%s()", location.file_name(), location.line(), location.column(), location.function_name());

    while(true) system::cpu::halt();
}