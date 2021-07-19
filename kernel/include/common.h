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

#define MAX_TASK_NUM 32
#define lock_t int

#define KMT_DEBUG

#define _Log(format, ...)                            \
    printf("\33[1;34m[%s,%d,%s] " format "\33[0m\n", \
        __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#define STACK_SIZE 8192

struct task {

    const char* name;
    enum task_status {
        SLEEP,
        DEAD,
    } status;

    Context* context;
    int cpu;

    char stack[STACK_SIZE];
};

struct spinlock {
    const char* name;
    int locked;
    int cpu;
};

struct semaphore {
    int value;
    spinlock_t lock;
    task_t* tasks[MAX_TASK_NUM];
    int head;
    int tail;
};

#define SEQ_MIN 0
#define SEQ_MAX 16

#define TRAP_HANDLER_MAX_NUM 16

struct trap_handler {
    int seq;
    int event;
    handler_t handler;
    enum trap_status {
        TRAP_EMPTY,
        TRAP_USED,
    } status;
};

typedef struct trap_handler trap_handler_t;

#endif