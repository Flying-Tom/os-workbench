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

struct co *co_group[CO_MAXNUM];
struct co *co_current;

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
    for (int i = 0; i < CO_MAXNUM; i++)
    {
        if (co_group[i] == NULL)
        {
            co_group[i] = malloc(sizeof(struct co));
            co_group[i]->name = (char *)name;
            co_group[i]->func = func;
            co_group[i]->arg = arg;
            co_group[i]->status = CO_NEW;

            return co_group[i];
        }
    }
    return NULL;
}

void co_wait(struct co *co)
{
    //printf("co_wait(%s) status:%d\n", co->name, co->status);

    co_current->status = CO_WAITING;
    co->waiter = co_current;
    while (co->status != CO_DEAD)
        co_yield();
    //printf("co_current->status:%d\n", co_current->status);
    co_current->status = CO_RUNNING;

    co->status = CO_UNDEFINE;
    //free(co);
}

void co_yield()
{
    //puts("co_yield");
    //printf("%s status:%d\n", co_group[0].name, co_group[0].status);
    int val = setjmp(co_current->context);
    //printf("%s %d  val:%d\n", co_current->name, co_current->status, val);
    if (val == 0)
    {
        int next_co_id, valid_co_num = 0;
        struct co *next_co = NULL;
        for (int i = 0; i < CO_MAXNUM; i++)
        {
            if (co_group[i] != NULL && (co_group[i]->status == CO_NEW || co_group[i]->status == CO_RUNNING))
                valid_co_num++;
        }

        next_co_id = rand() % valid_co_num;

        for (int i = 0; i < CO_MAXNUM; i++)
        {
            if (co_group[i]->status == CO_NEW || co_group[i]->status == CO_RUNNING)
            {
                if (next_co_id == 0)
                    co_current = co_group[i];
                next_co_id--;
            }
        }

        //printf("switch to: %s %d\n", next_co->name, next_co->status);
        assert(co_current);
        switch (co_current->status)
        {
        case CO_NEW:
            //puts("in");
            stack_switch_call((void *)(co_current->stack + STACK_SIZE - sizeof(uintptr_t)), coroutine_entry, (uintptr_t)co_current);
            break;
        case CO_RUNNING:
            longjmp(co_current->context, 1);
            break;
        default:
            //printf("%s %d\n", co_current->name, co_current->status);
            assert(0);
        }
    }
}

void __attribute__((constructor)) co_init()
{
    for (int i = 1; i < CO_MAXNUM; i++)
    {
        co_group[i] = malloc(sizeof(struct co));
        co_group[i]->status = CO_UNDEFINE;
    }

    co_group[0] = malloc(sizeof(struct co));
    co_group[0]->name = "main"; // main will be always waiting for other routines
    co_group[0]->status = CO_RUNNING;
    co_current = co_group[0];
    //puts("co_init finished");
}

void __attribute__((destructor)) co_end()
{
    //printf("co_group_cnt:%d\n", co_group_cnt);
}