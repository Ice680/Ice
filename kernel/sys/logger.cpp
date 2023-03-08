#include <stdio.h>
#include <sys/logger.hpp>

#define COLOR_BLUE "\033[1;34m"
#define COLOR_BLUE_BOLD "\033[1;34m"

#define COLOR_YELLOW "\033[0;33m"
#define COLOR_YELLOW_BOLD "\033[1;33m"

#define COLOR_RED "\033[0;31m"
#define COLOR_RED_BOLD "\033[1;31m"

#define COLOR_RESET "\033[0m"

namespace logger {
void vlog(log_level level, const char* fmt, va_list args) {
    const char* level_str;

    switch (level) {
        case log_level::INFO:
            level_str = COLOR_BLUE "info" COLOR_RESET;
            break;
        case log_level::WARN:
            level_str = COLOR_YELLOW "warn" COLOR_RESET;
            break;
        case log_level::ERROR:
            level_str = COLOR_RED "error" COLOR_RESET;
            break;
        default:
            level_str = "unknown";
            break;
    }

    printf("%s: ", level_str);
    vprintf(fmt, args);
    printf("\n");
}

void log(log_level level, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vlog(level, fmt, args);
    va_end(args);
}

void log_info(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    vlog(log_level::INFO, fmt, args);

    va_end(args);
}

void log_warn(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    vlog(log_level::WARN, fmt, args);

    va_end(args);
}

void log_error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    vlog(log_level::ERROR, fmt, args);

    va_end(args);
}
}  // namespace logger