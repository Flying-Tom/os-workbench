#include <pmm.h>

lock_t test_lk = LOCK_INIT();

static void *kalloc(size_t size)
{
    lock(&test_lk);
    void *ret = NULL;
    Log("kalloc: %d", size);
    size = binalign(size);
    if (size >= PAGE_SIZE)
        ret = buddy_alloc(size);
    else
        ret = slab_alloc(size);
    unlock(&test_lk);
    return ret;
}

static void kfree(void *ptr)
{
}

static void pmm_init()
{
    cpu_num = cpu_count();
    assert(cpu_num <= MAX_CPU_NUM);

    pm_start = (uintptr_t)heap.start;
    pm_end = (uintptr_t)heap.end;
    Log("pm_start:%p aligned pm_start:%p", pm_start, align(pm_start, PAGE_SIZE));
    Log("pm_end:%p", pm_end);
    pm_start = align(pm_start, PAGE_SIZE);

    slab_init(pm_start, (pm_start + pm_end) / 2);
    buddy_init((pm_start + pm_end) / 2, pm_end);

    assert((pm_end - pm_start) % PAGE_SIZE == 0);
    Log("pmm_init finished");
}

MODULE_DEF(pmm) = {
    .init = pmm_init,
    .alloc = kalloc,
    .free = kfree,
};
