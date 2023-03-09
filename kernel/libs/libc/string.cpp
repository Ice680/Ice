#include <string.h>

size_t strlen(const char* str) {
    size_t length = 0;
    while (str[length])
        length++;
    return length;
}

size_t strnlen(const char* str, size_t len) {
    size_t length = 0;
    while (str[length] && length < len)
        length++;
    return length;
}

int strcmp(const char* str1, const char* str2) {
    while (*str1 && *str2 && *str1 == *str2) {
        str1++;
        str2++;
    }

    return *str1 - *str2;
}

int strncmp(const char* str1, const char* str2, size_t len) {
    while (*str1 && *str2 && *str1 == *str2 && len--) {
        str1++;
        str2++;
    }

    return len == 0 ? 0 : *str1 - *str2;
}

char* strcpy(char* dest, const char* src) {
    char* ptr = dest;

    while (*src != '\0') {
        *dest = *src;
        dest++;
        src++;
    }

    *dest = '\0';

    return ptr;
}

char* strncpy(char* dest, const char* src, size_t len) {
    char* ptr = dest;

    while (*src != '\0' && len--) {
        *dest = *src;
        dest++;
        src++;
    }

    *dest = '\0';
    
    return ptr;
}

char* strcat(char* dest, const char* src) {
    char* ptr = dest + strlen(dest);

    while (*src != '\0')
        *ptr++ = *src++;

    *ptr = '\0';

    return dest;
}

char* strncat(char* dest, const char* src, size_t len) {
    char* ptr = dest + strlen(dest);

    while (*src != '\0' && len--)
        *ptr++ = *src++;

    *ptr = '\0';

    return dest;
}

char* strchr(const char* str, int ch) {
    while (*str && *str != ch)
        str++;

    return (char*)(ch == *str ? str : nullptr);
}

char* strstr(const char* str, const char* substr) {
    const char *a = str, *b = substr;
    while (true) {
        if (*b == 0)
            return (char*)str;
        if (*a == 0)
            return nullptr;

        if (*a++ != *b++) {
            a = ++str;
            b = substr;
        }
    }
}

void strrev(char* str) {
    char a;
    size_t len = strlen((const char*)str);

    for (size_t i = 0, j = len - 1; i < j; i++, j--) {
        a = str[i];
        str[i] = str[j];
        str[j] = a;
    }
}

int memcmp(const void* ptr1, const void* ptr2, size_t len) {
    const uint8_t* p1 = (const uint8_t*)ptr1;
    const uint8_t* p2 = (const uint8_t*)ptr2;

    for (size_t i = 0; i < len; i++)
        if (p1[i] != p2[i])
            return p1[i] < p2[2] ? -1 : 1;

    return 0;
}

void* memcpy(void* dest, const void* src, size_t len) {
    uint8_t* pdest = (uint8_t*)dest;
    const uint8_t* psrc = (const uint8_t*)src;

    for (size_t i = 0; i < len; i++)
        pdest[i] = psrc[i];

    return dest;
}

void* memmove(void* dest, const void* src, size_t len) {
    uint8_t* pdest = (uint8_t*)dest;
    const uint8_t* psrc = (const uint8_t*)src;

    if (src > dest) {
        for (size_t i = 0; i < len; i++)
            pdest[i] = psrc[i];
    } else if (src < dest) {
        for (size_t i = 0; i < len; i++)
            pdest[i - 1] = psrc[i - 1];
    }

    return dest;
}

void* memset(void* dest, int ch, size_t len) {
    uint8_t* p = (uint8_t*)dest;

    for (size_t i = 0; i < len; i++)
        p[i] = (uint8_t)ch;

    return dest;
}

void* memchr(const void* ptr, int ch, size_t len) {
    const uint8_t* src = (const uint8_t*)ptr;

    while (len-- > 0) {
        if (*src == ch)
            return (uint8_t*)src;
        src++;
    }

    return nullptr;
}