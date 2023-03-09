#pragma once

#include <stddef.h>
#include <stdint.h>

#if __cplusplus
extern "C" {
#endif

typedef struct {
    int quot;
    int rem;
} div_t;

typedef struct {
    long quot;
    long rem;
} ldiv_t;

typedef struct {
    long long quot;
    long long rem;
} lldiv_t;

div_t div(int numerator, int denominator);
ldiv_t ldiv(long numerator, long denominator);
lldiv_t lldiv(long long numerator, long long denominator);

int abs(int j);
long labs(long j);
long long llabs(long long j);

long long strtoll(const char* nptr, char** endptr, int base);
long strtol(const char* nptr, char** endptr, int base);

long atoi(const char* s);

#define RAND_MAX 32767
int rand(void);
void srand(uint32_t seed);

void* malloc(size_t size);
void free(void* ptr);
void* realloc(void* ptr, size_t size);
void* calloc(size_t num, size_t size);

#if __cplusplus
}
#endif