#ifndef COMMON_H
#define COMMON_H

#include <kernel.h>
#include <klib-macros.h>
#include <klib.h>
#include <limits.h>
#include <tool.h>

#define CPU_CUR (uint8_t)(cpu_current())
#define CPU_NUM (uint8_t)(cpu_count())
#define MAX_CPU_NUM 8

#define MAX_TASK_NUM 32
#define lock_t int

#define KMT_DEBUG

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