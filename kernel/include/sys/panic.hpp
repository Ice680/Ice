#pragma once

#include <arch/arch.hpp>
#include <cstdint>
#include <sys/logger.hpp>

namespace panic {
[[noreturn]] void panic(const char* file, int line, const char* func,
                        const char* message);
[[noreturn]] void panic(const char* message);

extern "C" [[noreturn]] void abort() noexcept;
}  // namespace panic