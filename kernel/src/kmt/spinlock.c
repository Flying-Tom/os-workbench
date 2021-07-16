#include <common.h>
#include <kmt.h>

/* XV6 spinlock */

int intena[MAX_CPU_NUM] = {};
int ncli[MAX_CPU_NUM] = {};

void spinmod_init()
{
    for (int i = 0; i < CPU_NUM; i++) {
        ncli[i] = 0;
        intena[i] = 1;
    }
}

void spinlock_init(spinlock_t* lk, const char* name)
{
    lk->name = name;
    lk->locked = 0;
    lk->cpu = -1;
}

void spinlock_acquire(spinlock_t* lk)
{
    spinlock_pushcli();
    if (spinlock_holding(lk))
        panic("acquire");

    while (atomic_xchg((int*)&lk->locked, 1))
        ;

    __sync_synchronize();
    lk->cpu = CPU_CUR;
}

void spinlock_release(spinlock_t* lk)
{
    if (!spinlock_holding(lk))
        panic("release");

    lk->cpu = -1;
    __sync_synchronize();
    asm volatile("movl $0,%0"
                 : "+m"(lk->locked)
                 :);
    spinlock_popcli();
}

uint8_t spinlock_holding(spinlock_t* lk)
{
    uint8_t ret = 0;
    spinlock_pushcli();
    ret = lk->locked && lk->cpu == CPU_CUR;
    spinlock_popcli();
    return ret;
}

void spinlock_pushcli()
{
    iset(false); // close interruption (cli())
    if (ncli[CPU_CUR] == 0)
        intena[CPU_CUR] = ienabled();

    ncli[CPU_CUR] += 1;
}

void spinlock_popcli()
{
    if (ienabled())
        panic("popcli - interruptible");

    ncli[CPU_CUR] -= 1;
    if (ncli[CPU_CUR] < 0)
        panic("popcli");

    if (ncli[CPU_CUR] == 0 && intena[CPU_CUR])
        iset(true); // start interruption (sti())
}