#include <common.h>

typedef struct
{
    int locked;
} lock_t;

#define LOCK_INIT() ((lock_t){.locked = 0})
void lock(lock_t *lk)
{
    while (atomic_xchg(&lk->locked, 1))
        ;
}
void unlock(lock_t *lk) { atomic_xchg(&lk->locked, 0); }

static lock_t lk = LOCK_INIT();

static void *kalloc(size_t size)
{
    void *ret = NULL;
    lock(&lk);
    unlock(&lk);
    return ret;
}

static void kfree(void *ptr)
{
}

#ifndef TEST
static void pmm_init()
{
    uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
    printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);
}
#else
static void pmm_init()
{
    char *ptr = malloc(HEAP_SIZE);
    heap.start = ptr;
    heap.end = ptr + HEAP_SIZE;
    printf("Got %d MiB heap: [%p, %p)\n", HEAP_SIZE >> 20, heap.start, heap.end);
}
#endif

MODULE_DEF(pmm) = {
    .init = pmm_init,
    .alloc = kalloc,
    .free = kfree,
};
