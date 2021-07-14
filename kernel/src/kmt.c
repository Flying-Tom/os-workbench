#include <kmt.h>

static void kmt_init()
{
    /*
    os->on_irq(INT_MIN, EVENT_NULL, kmt_context_save); // 总是最先调用
    os->on_irq(INT_MAX, EVENT_NULL, kmt_schedule); // 总是最后调用
    */
}

static int create(task_t* task, const char* name, void (*entry)(void* arg), void* arg)
{
    return 0;
}

static void teardown(task_t* task)
{
}

void spin_init(spinlock_t* lk, const char* name)
{
}

void spin_lock(spinlock_t* lk)
{
}

void spin_unlock(spinlock_t* lk)
{
}

void sem_init(sem_t* sem, const char* name, int value)
{
}

void sem_wait(sem_t* sem)
{
}

void sem_signal(sem_t* sem)
{
}

MODULE_DEF(kmt) = {
    .init = kmt_init,
    .create = create,
    .teardown = teardown,
    .spin_init = spin_init,
    .spin_lock = spin_lock,
    .spin_unlock = spin_unlock,
    .sem_init = sem_init,
    .sem_wait = sem_wait,
    .sem_signal = sem_signal,
};
