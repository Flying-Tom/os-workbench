#include <pmm.h>

static void* kalloc(size_t size)
{
    void* ret = NULL;
    Log("kalloc: %d", size);
    uint8_t order = calorder(size);
    if (order >= PAGE_ORDER)
        ret = buddy_alloc(order);
    else
        ret = slab_alloc(max(0, order - 5));
    //assert((uintptr_t)ret % binalign(size) == 0);
    return ret;
}

static void kfree(void* ptr)
{
    if ((uintptr_t)ptr & PAGE_RMASK || (ptr >= slab_start && ptr < slab_end))
        slab_free(ptr);
    else
        buddy_free(ptr);
}

static void pmm_init()
{
    void *pm_start, *pm_end, *pm_cur;
    uintptr_t pm_size;

    assert(CPU_NUM <= MAX_CPU_NUM);

    pm_start = heap.start;
    pm_end = heap.end;
    pm_size = (uintptr_t)pm_end - (uintptr_t)pm_start;

    Log("pm_start:%p pm_end:%p pm_size:%d", pm_start, pm_end, pm_size);

    uintptr_t pm_cache_size = (pm_size / 16 * 3 / CPU_NUM) & PAGE_LMASK;

    slab_start = (void*)((uintptr_t)(pm_start + PAGE_SIZE - 1) & PAGE_LMASK);
    pm_cur = slab_start;
    for (int i = 0; i < CPU_NUM; i++) {
        slab_init(i, pm_cur, pm_cache_size);
        pm_cur += pm_cache_size;
    }
    slab_end = pm_cur;

    buddy_init(pm_cur, pm_end);

    Log("pmm_init finished");
    //assert(0);
}

static void* kalloc_safe(size_t size)
{
    bool i = ienabled();
    iset(false);
    void* ret = kalloc(size);
    if (i)
        iset(true);
    return ret;
}

static void kfree_safe(void* ptr)
{
    int i = ienabled();
    iset(false);
    kfree(ptr);
    if (i)
        iset(true);
}

MODULE_DEF(pmm) = {
    .init = pmm_init,
    .alloc = kalloc_safe,
    .free = kfree_safe,
};
