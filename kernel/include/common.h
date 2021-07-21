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

#define MAX_TASK_NUM 24
#define lock_t int

#define KMT_DEBUG

#define STACK_SIZE 4096

struct task {

    const char* name;
    enum task_status {
        TASK_EMPTY,
        TASK_RUNNING,
        TASK_WAITTING,
        TASK_DEAD,
    } status;
    int running, pause, id;
    Context* context;

    char* stack;
};

struct spinlock {
    const char* name;
    int locked;
    int cpu;
};

#define MAX_SEM_TASK_NUM 32

struct semaphore {
    int value;
    spinlock_t lock;
    task_t* tasks[MAX_SEM_TASK_NUM];
    int head;
    int tail;
};

//#define INT_MIN INT32_MIN
//#define INT_MAX INT32_MAX

struct trap {
    int seq;
    int event;
    handler_t handler;
    struct trap* next;
};

typedef struct trap trap_t;

#endif