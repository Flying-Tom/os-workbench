#include "co.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <setjmp.h>

#define STACK_SIZE 64 * 1024
#define CO_MAXNUM 128

enum co_status
{
    CO_NEW,
    CO_RUNNING,
    CO_WAITING,
    CO_DEAD
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
} co_group[CO_MAXNUM];

struct co *co_current;

static inline void stack_switch_call(void *sp, void *entry, uintptr_t arg)
{
    asm volatile(
#if __x86_64__
        "movq %0, %%rsp; movq %2, %%rdi; jmp *%1" ::"b"((uintptr_t)sp), "d"(entry), "a"(arg)
#else
        "movl %0, %%esp; movl %2, 4(%0); jmp *%1" ::"b"((uintptr_t)sp - 8), "d"(entry), "a"(arg)
#endif
    );
}

void coroutine_entry(struct co *co)
{
    co->status = CO_RUNNING;
    co->func(co->arg);
    if (co->waiter)
        co->waiter->status = CO_RUNNING;
    co->status = CO_DEAD;
    co_yield();
}

struct co *co_start(const char *name, void (*func)(void *), void *arg)
{
    for (int i = 0; i < CO_MAXNUM; i++)
    {
        if (co_group[i].status == CO_DEAD)
        {
            co_group[i].name = (char *)name;
            co_group[i].func = func;
            co_group[i].arg = arg;
            co_group[i].status = CO_NEW;
            return &co_group[i];
        }
    }
    return NULL;
}

void co_wait(struct co *co)
{
    co_current->status = CO_WAITING;
    co->waiter = co_current;
    while (co->status != CO_DEAD)
        co_yield();
    co_current->status = CO_RUNNING;
}

void co_yield()
{
    if (setjmp(co_current->context) == 0)
    {
        do
        {
            co_current = &co_group[rand() % CO_MAXNUM];
        } while (co_current->status > CO_RUNNING);

        switch (co_current->status)
        {
        case CO_NEW:
            stack_switch_call((void *)(co_current->stack + STACK_SIZE - sizeof(uintptr_t)), coroutine_entry, (uintptr_t)co_current);
            break;
        case CO_RUNNING:
            longjmp(co_current->context, 1);
            break;
        default:
            break;
        }
    }
}

void __attribute__((constructor)) co_init()
{
    for (int i = 1; i < CO_MAXNUM; i++)
        co_group[i].status = CO_DEAD;

    co_group[0].name = "main"; // main will be always waiting for other routines
    co_group[0].status = CO_RUNNING;
    co_current = &co_group[0];
}
