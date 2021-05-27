#include <pmm.h>

lock_t test_lk = LOCK_INIT();

static void *kalloc(size_t size)
{
    void *ret = NULL;
    Log("kalloc: %d", size);
    size = binalign(size);
    if (size >= PAGE_SIZE)
        ret = buddy_alloc(size);
    else
        ret = slab_alloc(size);
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
    pm_start = align(pm_start, PAGE_SIZE);
    pm_end = (uintptr_t)heap.end;
    pm_size = pm_end - pm_start;

    Log("pm_start:%p pm_end:%p", pm_start, pm_end);
    Log("pm_size:%d", pm_size);

    uintptr_t pm_interval = align(pm_start + pm_size / 8 * 6 / cpu_count(), PAGE_SIZE);

    slab_init(pm_start, pm_interval);
    buddy_init(pm_interval, pm_end);

    assert((pm_end - pm_start) % PAGE_SIZE == 0);
    Log("pmm_init finished");
}

MODULE_DEF(pmm) = {
    .init = pmm_init,
    .alloc = kalloc,
    .free = kfree,
};
