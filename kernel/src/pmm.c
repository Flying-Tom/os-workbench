#include <pmm.h>

void *pm_start, *pm_end, *pm_cur;
uintptr_t pm_size;

static void *kalloc(size_t size)
{
    void *ret = NULL;
    Log("kalloc: %d", size);
    if (size > PAGE_SIZE)
        ret = buddy_alloc(size);
    else
        ret = slab_alloc(size);
    assert((uintptr_t)ret % binalign(size) == 0);
    return ret;
}

static void kfree(void *ptr)
{
}

static void pmm_init()
{
    assert((cpu_num = cpu_count()) <= MAX_CPU_NUM);

    pm_start = heap.start;
    pm_end = heap.end;
    pm_size = (uintptr_t)pm_end - (uintptr_t)pm_start;

    Log("pm_start:%p pm_end:%p pm_size:%d", pm_start, pm_end, pm_size);

    size_t pm_cache_size = (pm_size / 8 * 3 / cpu_num) & PAGE_LMASK;
    Log("pm_cache_size:%d", pm_cache_size);

    slab_start = (void *)((uintptr_t)(pm_start + PAGE_SIZE - 1) & PAGE_LMASK);
    pm_cur = slab_start;
    for (int i = 0; i < cpu_num; i++)
    {
        slab_init(i, pm_cur, pm_cache_size);
        pm_cur += pm_cache_size;
    }
    slab_end = pm_cur;

    buddy_init(pm_cur, pm_end);

    Log("pmm_init finished");
}

MODULE_DEF(pmm) = {
    .init = pmm_init,
    .alloc = kalloc,
    .free = kfree,
};
