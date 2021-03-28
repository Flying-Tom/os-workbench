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
    CO_UNDEFINE,
    CO_NEW,
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
}; // root coroutine

struct co *co_current;
struct co *co_group[CO_MAXNUM];

int co_group_cnt;

static inline void stack_switch_call(void *sp, void *entry, uintptr_t arg)
{
    // restore the stack pointer, and move arg to %rdi( the first parameter register), then call the function "entry"
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
    //co_group_cnt--; can't be there because the list need co_group_cnt to determine the specific element, and here co isn't released yet
    if (co->waiter)
        co->waiter->status = CO_RUNNING;
    co_yield();
}

struct co *co_start(const char *name, void (*func)(void *), void *arg)
{
    struct co *new_co = malloc(sizeof(struct co));
    new_co->name = (char *)name;
    new_co->func = func;
    new_co->arg = arg;
    new_co->status = CO_NEW;

    co_group[co_group_cnt++] = new_co;
    return new_co;
}

void co_wait(struct co *co)
{
    //printf("co_wait(%s) status:%d\n", co->name, co->status);
    if (co->status != CO_DEAD)
    {
        co_current->status = CO_WAITING;
        co->waiter = co_current;
        while (co->status != CO_DEAD)
            co_yield();
        //printf("co_current->status:%d\n", co_current->status);
        co_current->status = CO_RUNNING;
    }
    else
    {
        *co = *co_group[co_group_cnt];
        free(co_group[co_group_cnt--]);
    }
}

void co_yield()
{
    //puts("co_yield");
    //printf("%s status:%d\n", co_group[0]->name, co_group[0]->status);
    int val = setjmp(co_current->context);
    //printf("%s %d  val:%d\n", co_current->name, co_current->status, val);
    if (val == 0)
    {
        struct co *next_co = NULL;

        do
        {
            int next_co_id = rand() % co_group_cnt;
            next_co = co_group[next_co_id];
        } while (next_co->status == CO_RUNNING || next_co->status == CO_NEW);

        assert(next_co != NULL);
        //printf("switch to: %s %d\n", next_co->name, next_co->status);
        co_current = next_co;
        switch (co_current->status)
        {
        case CO_NEW:
            stack_switch_call((void *)(co_current->stack + STACK_SIZE), coroutine_entry, (uintptr_t)co_current);
            //puts("out");
            break;
        case CO_RUNNING:
            longjmp(co_current->context, 1);
            break;
        default:
            //printf("%s %d\n", co_current->name, co_current->status);
            assert(0);
            break;
        }
    }
    else
    {
        // return from longjmp
        // printf("co_group[0]->status:%d\n", co_group[0]->status);
        return;
    }
}

void __attribute__((constructor)) co_init()
{
    co_group[0]->name = "main"; // main will be always waiting for other routines
    co_group[0]->status = CO_RUNNING;
    co_group_cnt = 1;
}