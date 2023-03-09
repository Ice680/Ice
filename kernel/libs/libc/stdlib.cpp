#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <memory/memory.hpp>

unsigned long long cseed = 1;

int abs(int j) {
    return (j < 0 ? -j : j);
}

long labs(long j) {
    return (j < 0 ? -j : j);
}

long long llabs(long long j) {
    return (j < 0 ? -j : j);
}

long atoi(const char* s) {
    long num = 0;
    int i = 0;

    while (s[i] && (s[i] >= '0' && s[i] <= '9')) {
        num = num * 10 + (s[i] - '0');
        i++;
    }

    return num;
}

long long strtoll(const char* nptr, char** endptr, int base) {
    const char* s = nptr;
    long long ret_value = 0;
    bool is_neg = false;

    while (*s == ' ')
        s++;

    if (*s == '-') {
        is_neg = true;
        s++;
    }

    // if base == 0 and detect 0x at the start of string put it at 16
    if ((base == 0 || base == 16) && s[0] == '0' &&
        (s[1] == 'x' || s[1] == 'X')) {
        s += 2;
        base = 16;
    }

    if (base == 0)
        base = 10;

    char* start = (char*)s;
    while (isalnum(*s)) {
        if (base == 10) {
            if (isdigit(*s))
                ret_value = (*s - '0') * base;
            else
                break;
        } else if (base == 16) {
            if (isdigit(*s))
                ret_value = (*s - '0') * base;
            else
                ret_value = ((tolower(*s) - 'a') + 10) * base;
        }
        s++;
    }

    if (is_neg)
        ret_value *= -1;

    if (endptr != 0)
        *endptr = start;

    return ret_value;
}

long strtol(const char* nptr, char** endptr, int base) {

    const char* s = nptr;
    long ret_value = 0;
    bool is_neg = false;

    while (*s == ' ')
        s++;

    if (*s == '-') {
        is_neg = true;
        s++;
    }

    // if base == 0 and detect 0x at the start of string put it at 16
    if ((base == 0 || base == 16) && s[0] == '0' &&
        (s[1] == 'x' || s[1] == 'X')) {
        s += 2;
        base = 16;
    }

    if (base == 0)
        base = 10;

    char* start = (char*)s;

    while (isalnum(*s)) {
        if (base == 10) {
            if (isdigit(*s))
                ret_value = (*s - '0') * base;
            else
                break;
        } else if (base == 16) {
            if (isdigit(*s))
                ret_value = (*s - '0') * base;
            else
                ret_value = ((tolower(*s) - 'a') + 10) * base;
        }
        s++;
    }

    if (is_neg)
        ret_value *= -1;

    if (endptr != 0)
        *endptr = start;

    return ret_value;
}

int rand() {
    cseed = cseed * 1103515245 + 12345;
    return (uint32_t)(cseed / 65536) % 32768;
}

void srand(uint32_t seed) {
    cseed = seed;
}

void* malloc(size_t size) {
    return memory::allocator->malloc<void*>(size);
}

void free(void* ptr) {
    return memory::allocator->free(ptr);
}

void* realloc(void* ptr, size_t size) {
    return memory::allocator->realloc<void*>(ptr, size);
}

void* calloc(size_t num, size_t size) {
    return memory::allocator->calloc<void*>(num, size);
}

div_t div(int numerator, int denominator) {
    div_t result;
    result.quot = numerator / denominator;
    result.rem = numerator % denominator;
    return result;
}

ldiv_t ldiv(long numerator, long denominator) {
    ldiv_t result;
    result.quot = numerator / denominator;
    result.rem = numerator % denominator;
    return result;
}

lldiv_t lldiv(long long numerator, long long denominator) {
    lldiv_t result;
    result.quot = numerator / denominator;
    result.rem = numerator % denominator;
    return result;
}