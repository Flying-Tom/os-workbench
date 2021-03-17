#include "co.h"
#include <stdlib.h>
#include <stdint.h>
#include <setjmp.h>

#define STACK_SIZE 64 * 1024

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

struct co *co_start(const char *name, void (*func)(void *), void *arg)
{
    struct co *new_co = malloc(sizeof(struct co));
    *new_co->name = *name;
    new_co->func = func;
    new_co->arg = arg;

    return new_co;
}

void co_wait(struct co *co)
{
    (*co)();
    free(co);
}

void co_yield()
{
}
