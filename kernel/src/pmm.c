#include <pmm.h>

void *pm_start, pm_end;

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

    pm_start = heap.start;
    //pm_start = align(pm_start, PAGE_SIZE);
    pm_end = heap.end;
    pm_size = (uintptr_t)pm_end - (uintptr_t)pm_start;

    Log("pm_start:%p pm_end:%p pm_size:%d", pm_start, pm_end, pm_size);

    size_t pm_cache_size = 0;
    for (int i = 0; i < CPU_CUR; i++)
    {
        slab_init(i, pm_start, pm_cache_size);
        pm_start += pm_cache_size;
    }
    slab_end = pm_start;

    buddy_init(pm_start, pm_end);

    Log("pmm_init finished");
}

MODULE_DEF(pmm) = {
    .init = pmm_init,
    .alloc = kalloc,
    .free = kfree,
};
