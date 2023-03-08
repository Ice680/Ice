#include <source_location>
#include <sys/logger.hpp>
#include <sys/panic.hpp>
#include <stdio.h>

namespace panic {
[[noreturn]] void panic(const char* file, int line, const char* func,
                        const char* message) {
    printf("\n");
    logger::log_error("%s", message);
    logger::log_error("File: %s", file);
    logger::log_error("Line: %s", line);
    logger::log_error("Function: %s", func);
    logger::log_error("System halted!");

    system::cpu::halt();
}

[[noreturn]] void panic(const char* message) {
    printf("\n");
    logger::log_error("%s", message);
    logger::log_error("System halted!");

    system::cpu::halt();
}

extern "C" [[noreturn]] void abort() noexcept {
    panic("abort()");
}
}  // namespace panic