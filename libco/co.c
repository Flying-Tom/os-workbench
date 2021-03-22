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
    struct co *prev;

    jmp_buf context;
    uint8_t stack[STACK_SIZE];
} co_main; // root coroutine

struct co *co_list_head = &co_main;
struct co *co_current = &co_main;

int co_group_cnt = 1;

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
    co_group_cnt--;
    //co_yield();
}

void coroutine_switch(struct co *co)
{
    co_current = co;
    //uintptr_t ptrtemp = (uintptr_t)(((uintptr_t)(co->stack + STACK_SIZE) >> 4) << 4);
    switch (co->status)
    {
    case CO_NEW:
        stack_switch_call((void *)(co->stack + STACK_SIZE), coroutine_entry, (uintptr_t)co);
        //stack_switch_call((void*)((uintptr_t)((((uintptr_t)co->stack + STACK_SIZE) >> 4) << 4)), coroutine_entry, (uintptr_t)co);
        puts("out");
        break;
    case CO_RUNNING:
        //puts("longjmp");
        longjmp(co_current->context, 1);
        break;
    default:
        //printf("%s %d\n", co->name, co->status);
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

    new_co->prev = co_list_head;
    co_list_head = new_co;

    co_group_cnt++;
    //printf("%s\n", co_list_head->name);

    return new_co;
}

void co_wait(struct co *co)
{
    printf("co_wait(%s)\n", co->name);
    assert(co->status > 0);
    if (co->status != CO_DEAD)
    {
        co_current->status = CO_WAITING;
        //printf("co_current:%s %d\n", co_current->name, co_current->status);
        co->waiter = co_current;
        /*
        while (co->status != CO_DEAD)
        {
            co_yield();
        }*/
        co_yield();
        assert(co->status == CO_DEAD);
        puts("Out!");
        co_current->status = CO_RUNNING;
    }

    struct co *co_temp = co_list_head;
    while (co_temp->prev != co)
        co_temp = co_temp->prev;

    co_temp->prev = co->prev;
    free(co);
}

void co_yield()
{
    int val = setjmp(co_current->context);
    printf("%s %d  val:%d\n", co_current->name, co_current->status, val);
    if (val == 0)
    {
        // start to switch coruntine
        int next_co_id;
        struct co *next_co;

        do
        {
            next_co_id = rand() % co_group_cnt + 1;
            //printf("next_co_id:%d\n", next_co_id);
            next_co = co_list_head;
            //printf("%s %d\n", next_co->name, next_co->status);
            //printf("%s %d\n", next_co->prev->name, next_co->prev->status);
            //assert(0);
            while (--next_co_id)
            {
                next_co = next_co->prev;
            }
            //printf("next_co->status:%d\n", next_co->status);
            //printf("co_group_cnt:%d\n", co_group_cnt);
        } while (next_co->status != CO_RUNNING && next_co->status != CO_NEW);
        //printf("switch to: %s %d\n", next_co->name, next_co->status);
        coroutine_switch(next_co);
    }
    else
    {
        // return from longjmp
        // printf("co_main.status:%d\n", co_main.status);
        return;
    }
}

void __attribute__((constructor)) beforemain()
{
    //strcpy(co_main.name, "main");
    co_main.status = CO_RUNNING;
}