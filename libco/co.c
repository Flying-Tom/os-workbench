#include "co.h"
#include <stdlib.h>
#include <stdint.h>
#include <setjmp.h>
#include <string.h>
#include <assert.h>

#define STACK_SIZE 64 * 1024
#define CO_MAXNUM 128

enum co_status
{
    CO_NEW = 1,
    CO_RUNNING,
    CO_WAITING,
    CO_DEAD,
};

struct co
{
    char *name;
    void (*func)(void *);
    void *arg;

    enum co_status status;
    struct co *waiter;
    jmp_buf context;
    uint8_t stack[STACK_SIZE];
};

struct co *co_current, *co_group[CO_MAXNUM];
int co_group_cnt;

static inline void stack_switch_call(void *sp, void *entry, uintptr_t arg)
{
    asm volatile(
#if __x86_64__
        "movq %0, %%rsp; movq %2, %%rdi; jmp *%1"
        :
        : "b"((uintptr_t)sp), "d"(entry), "a"(arg)
#else
        "movl %0, %%esp; movl %2, 4(%0); jmp *%1"
        :
        : "b"((uintptr_t)sp - 8), "d"(entry), "a"(arg)
#endif
    );
}

struct co *co_start(const char *name, void (*func)(void *), void *arg)
{
    struct co *new_co = malloc(sizeof(struct co));
    *new_co->name = *name;
    strcpy(new_co->name, name);
    new_co->func = func;
    new_co->arg = arg;
    new_co->status = CO_NEW;

    return new_co;
}

void co_wait(struct co *co)
{
    co_current->status = CO_WAITING;
    co->waiter = co_current;
    stack_switch_call(co->stack, co->func, (uintptr_t)co->arg);
    co->status = CO_DEAD;

    co_current->status = CO_RUNNING;
    free(co);
}

void co_yield()
{
    int val = setjmp(co_current->context);
    if (val == 0)
    {
        // start to switch coruntine
        int next_co_id;
        do
        {
            next_co_id = rand() % co_group_cnt;
        } while (co_group[next_co_id]->status == CO_DEAD);
        co_current = co_group[next_co_id];
        longjmp(co_current->context, 1);
    }
    else
    {
        // return from longjmp
        return;
    }
}
