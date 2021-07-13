#ifndef COMMON_H
#define COMMON_H

#include <kernel.h>
#include <klib.h>
#include <klib-macros.h>

#define KB *(1L << 10)
#define MB *(1L << 20)
#define GB *(1L << 30)

#define max(a, b) ((a > b) ? (a) : (b))
#define min(a, b) ((a < b) ? (a) : (b))

#define lock_t int

inline void lock(lock_t *lk)
{
    while (atomic_xchg(lk, 1))
        ;
}
inline void unlock(lock_t *lk)
{
    atomic_xchg(lk, 0);
}

//#define DEBUG

#define _Log(format, ...)                            \
    printf("\33[1;34m[%s,%d,%s] " format "\33[0m\n", \
           __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#ifdef DEBUG
#define Log(...) _Log(__VA_ARGS__)
#else
#define Log(...)
#endif

#endif