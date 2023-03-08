#pragma once

namespace logger {
enum class log_level { INFO, WARN, ERROR };

void log(log_level level, const char* fmt, ...);

void log_info(const char* text, ...);
void log_warn(const char* text, ...);
void log_error(const char* text, ...);
}  // namespace logger