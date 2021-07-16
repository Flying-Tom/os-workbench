#ifndef COMMON_H
#define COMMON_H

#include <kernel.h>
#include <klib-macros.h>
#include <klib.h>
#include <limits.h>

#define KB *(1L << 10)
#define MB *(1L << 20)
#define GB *(1L << 30)

#define max(a, b) ((a > b) ? (a) : (b))
#define min(a, b) ((a < b) ? (a) : (b))

#define CPU_CUR (uint8_t)(cpu_current())
#define CPU_NUM (uint8_t)(cpu_count())
#define MAX_CPU_NUM 8

#define MAX_TASK_NUM 8

#define DEBUG

#define _Log(format, ...)                            \
    printf("\33[1;34m[%s,%d,%s] " format "\33[0m\n", \
        __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#ifdef DEBUG
#define Log(...) _Log(__VA_ARGS__)
#else
#define Log(...)
#endif

typedef struct task task_t;
typedef struct spinlock spinlock_t;
typedef struct semaphore sem_t;

#endif