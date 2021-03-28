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
struct co co_group[CO_MAXNUM];

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
    //puts("coroutine_entry");
    //printf("%s\n", co->name);
    co->status = CO_RUNNING;
    co->func(co->arg);
    co->status = CO_DEAD;
    //puts("func finished");
    //co_group_cnt--; can't be there because the list need co_group_cnt to determine the specific element, and here co isn't released yet
    if (co->waiter)
        co->waiter->status = CO_RUNNING;
    co_yield();
}

struct co *co_start(const char *name, void (*func)(void *), void *arg)
{
    co_group[co_group_cnt].name = (char *)name;
    co_group[co_group_cnt].func = func;
    co_group[co_group_cnt].arg = arg;
    co_group[co_group_cnt].status = CO_NEW;
    //puts("co_start finished");
    return &co_group[co_group_cnt++];
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

    //*co = *co_group[co_group_cnt];
    //if (co != co_group[co_group_cnt - 1])
    memmove(co, co_group[co_group_cnt - 1], sizeof(struct co));
    co_group_cnt--;
    //puts("free");
    //free(co_group[co_group_cnt--]);
    //printf("co_group_cnt:%d\n", co_group_cnt);
    //puts("free end");
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
            next_co = &co_group[next_co_id];
        } while (next_co->status != CO_RUNNING && next_co->status != CO_NEW);

        assert(next_co != NULL);
        //printf("switch to: %s %d\n", next_co->name, next_co->status);
        co_current = next_co;
        switch (co_current->status)
        {
        case CO_NEW:
            //puts("co_new");
            //printf("co_current->stack:%p\n", co_current->stack);
            //printf("(((uintptr_t)co_current->stack >> 4) << 4):%p\n", (void *)(((uintptr_t)co_current->stack >> 4) << 4));
            stack_switch_call((void *)((uintptr_t)co_current->stack + STACK_SIZE - sizeof(uintptr_t)), coroutine_entry, (uintptr_t)co_current);
            //stack_switch_call((void *)((((uintptr_t)co_current->stack >> 4) << 4) + STACK_SIZE), coroutine_entry, (uintptr_t)co_current);
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
    co_group[0].name = "main"; // main will be always waiting for other routines
    co_group[0].status = CO_RUNNING;
    co_current = &co_group[0];
    co_group_cnt = 1;
}