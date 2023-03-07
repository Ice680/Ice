#pragma once
#include <stddef.h>
#include <stdint.h>

extern "C" {
size_t strlen(const char* str);
size_t strnlen(const char* str, size_t len);

int strcmp(const char* str1, const char* str2);
int strncmp(const char* str1, const char* str2, size_t len);

char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t len);

char* strcat(char* dest, const char* src);
char* strncat(char* det, const char* src, size_t len);

char* strchr(const char* str, int ch);
char* strstr(const char* str, const char* substr);
void strrev(char* str);

int memcmp(const void* ptr1, const void* ptr2, size_t len);
void* memcpy(void* dest, const void* src, size_t len);
void* memmove(void* dest, const void* src, size_t len);
void* memset(void* dest, int ch, size_t len);
void* memchr(const void* ptr, int ch, size_t len);
}