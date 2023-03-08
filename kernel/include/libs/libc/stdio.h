#pragma once

#include <stdarg.h>
#include <stddef.h>

void putchar(char c);

int printf(const char* format, ...);
int vprintf(const char* format, va_list arg);

int sprintf(char* s, const char* format, ...);
int vsprintf(char* s, const char* format, va_list arg);

int snprintf(char* s, size_t count, const char* format, ...);
int vsnprintf(char* s, size_t count, const char* format, va_list arg);

int fctprintf(void (*out)(char c, void* extra_arg), void* extra_arg,
              const char* format, ...);
int vfctprintf(void (*out)(char c, void* extra_arg), void* extra_arg,
               const char* format, va_list arg);