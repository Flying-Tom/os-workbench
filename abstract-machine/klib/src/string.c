#include <klib.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char* s)
{
    size_t cnt = 0;
    while (*s++)
        cnt++;
    return cnt;
}

char* strcpy(char* dst, const char* src)
{
    assert(dst != NULL && src != NULL);
    char* ret = dst;
    while ((*dst++ = *src++) != '\0')
        ;
    return ret;
}

char* strncpy(char* dst, const char* src, size_t n)
{
    char* ret = dst;
    while ((*dst++ = *src++)) {
        if (!n--)
            return ret;
    }
    while (n--)
        *dst++ = '\0';
    return ret;
}

char* strcat(char* dst, const char* src)
{
    char* ret = dst;
    while (*dst)
        dst++;
    while ((*dst++ = *src++))
        ;
    return ret;
}

int strcmp(const char* s1, const char* s2)
{
    while (*s1 && *s2 && *s1 == *s2) {
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

int strncmp(const char* s1, const char* s2, size_t n)
{
    while (n--) {
        if (*s1++ != *s2++)
            return *(s1 - 1) - *(s2 - 1);
    }
    return 0;
}

void* memset(void* v, int c, size_t n)
{
    if (v == NULL || n < 0)
        return NULL;
    char* p = (char*)v;
    while (n--)
        *p++ = c;
    return v;
}

void* memmove(void* dst, const void* src, size_t n)
{
    char* dsttmp = dst;
    const char* srctmp = src;
    if (dsttmp < srctmp) {
        while (n--)
            *dsttmp++ = *srctmp++;
    } else {
        const char* s = srctmp + n - 1;
        char* d = dsttmp + n - 1;
        while (n--)
            *d-- = *s--;
    }
    return dst;
}

void* memcpy(void* out, const void* in, size_t n)
{
    char* dp = out;
    const char* sp = in;
    while (n--)
        *dp++ = *sp++;
    return out;
}

int memcmp(const void* s1, const void* s2, size_t n)
{
    char* str1 = (char*)s1;
    char* str2 = (char*)s2;
    while (n) {
        if (*str1 == '\0' || *str2 == '\0')
            break;
        if (*str1 == *str2) {
            str1++;
            str2++;
        }
        n--;
    }
    if (n == 0)
        return 0;
    if (*str1 > *str2)
        return 1;
    if (*str1 < *str2)
        return -1;
    return 0;
}

#endif