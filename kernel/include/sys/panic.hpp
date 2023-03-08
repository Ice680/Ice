#pragma once

#include <source_location>

[[noreturn]] void panic(
    const char* error, void* stacktrace_addr = nullptr,
    const std::source_location location = std::source_location());