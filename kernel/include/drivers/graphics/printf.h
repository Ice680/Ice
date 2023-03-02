/** 
 * Copied from https://github.com/eyalroz/printf with modifications
 */
#pragma once

#include <stdarg.h>
#include <stddef.h>

/**
 * Prints/send a single character to stdio
 * 
 * @param c the single character to print
 */
void putchar(char c);

/**
 * An implementation of the C standard's printf/vprintf
 * 
 * @param format A string specifying the format of the output, with %-marked specifiers 
 *  of how to interpret additional arguments
 * @param arg additional arguments to the function, one for each %-specifier in @p format string
 * @return int The number of characters written into @p s, not counting the termination null character
 */
int printf(const char* format, ...);
int vprintf(const char* format, va_list arg);

/**
 * An implementation of the C standard's sprintf/vsprintf
 * 
 * @param s An array in which to store the formatted string.
 * @param format A string specifying the format of the output, with %-marked specifiers
 *  of how to interpret additional arguments
 * @param arg Additional arguments to the function, one for each specifier in @p format
 * @return The number of characters written into @p s, not counting the terminating null character
 */
int sprintf(char* s, const char* format, ...);
int vsprintf(char* s, const char* format, va_list arg);

/**
 * An implementation of the C standard's snprintf/vsnprintf
 * 
 * @param s An array in which to store the formatted string.
 * @param count The maximum number of characters to write to the array, including a terminating null charater
 * @param arg Additional arguments to the function, one for each specifier in @p format
 * @return The number of characters the could have been written into @p s, not counting the terminating null character.
 *  a value >= @p count -> truncation; value > 0 && value < @p count -> successful 
 */
int snprintf(char* s, size_t count, const char* format, ...);
int vsnprintf(char* s, size_t count, const char* format, va_list arg);

/**
 * printf/vprintf with user-specified output function
 *
 * @param out An output function which takes one character and a type-erased additional parameters
 * @param extra_arg The type-erased argument to pass to the output function @p out with each call
 * @param format A string specifying the format of the output, with %-marked specifiers of how to interpret
 * additional arguments.
 * @param arg Additional arguments to the function, one for each specifier in @p format
 * @return The number of characters for which the output f unction was invoked, not counting the terminating null character
 */
int fctprintf(void (*out)(char c, void* extra_arg), void* extra_arg,
              const char* format, ...);
int vfctprintf(void (*out)(char c, void* extra_arg), void* extra_arg,
               const char* format, va_list arg);