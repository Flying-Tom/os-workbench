#include "co.h"
#include <stdio.h>
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
    struct co *prev;
    jmp_buf context;
    uint8_t stack[STACK_SIZE];
} co_root;

struct co *co_list_head = &co_root;

struct co *co_current, co_group[CO_MAXNUM];
int co_group_cnt;

static inline void stack_switch_call(void *sp, void *entry, uintptr_t arg)
{
    // restore the stack pointer, and move arg to %rdi( the fist parameter register), then call the function "entry"
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

void coroutine_entry(struct co *co)
{
    co->status = CO_RUNNING;
    co->func(co->arg);
    co->status = CO_DEAD;
}

void coroutine_switch(struct co *co)
{
    co_current = co;
    switch (co->status)
    {
    case CO_NEW:
        stack_switch_call((void *)(co->stack), coroutine_entry, (uintptr_t)co->arg);
        break;
    case CO_RUNNING:
        longjmp(co_current->context, 1);
        break;
    default:
        assert(0);
        break;
    }
}

struct co *co_start(const char *name, void (*func)(void *), void *arg)
{

    struct co *new_co = malloc(sizeof(struct co));
    new_co->name = (char *)name;
    new_co->func = func;
    new_co->arg = arg;
    new_co->status = CO_NEW;

    co_group_cnt++;
    *(new_co->prev) = *co_list_head;
    co_list_head = new_co;

    return new_co;
}

void co_wait(struct co *co)
{
    printf("co_wait(%s)\n", co->name);
    assert(co->status > 0);
    if (co->status != CO_DEAD)
    {
        co_current->status = CO_WAITING;
        co->waiter = co_current;
        puts("while");
        while (co->status != CO_DEAD)
        {
            co_yield();
        }
        co_current->status = CO_RUNNING;
    }
    free(co);
}

void co_yield()
{
    int val = setjmp(co_current->context);
    if (val == 0)
    {
        // start to switch coruntine
        int next_co_id;
        struct co *next_co;

        do
        {
            next_co_id = rand() % co_group_cnt + 1;
            next_co = co_list_head;
            while (next_co_id--)
            {
                next_co = next_co->prev;
            }

        } while (next_co->status != CO_RUNNING);
        coroutine_switch(next_co);
    }
    else
    {
        // return from longjmp
        return;
    }
}
