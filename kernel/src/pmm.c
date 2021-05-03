#include <pmm.h>

static bool page_full(page_header *cur)
{
    if (cur == NULL)
        return true;
    uint64_t i, tmp = 0;
    for (i = 0; i < 64; i++)
    {
        if (!BITMAP_FULL(cur, i))
        {
            tmp = cur->bitmap[i];
            break;
        }
    }
    i = i << 8;
    while (tmp & 1)
    {
        tmp >>= 1;
        i++;
    }

    return (i + 1 >= PAGE_SIZE / (1 << cur->slab_type));
}

static void *slab_alloc(size_t size)
{
    void *ret = NULL;
    uint8_t type = 0, cur_cpu_id = 0;
    cur_cpu_id = cpu_current();

    type = max(3, log(size));
    size = 1 << type;

    Log("type:%d size:%d", type, size);

    Cache *object_cache = &cache[cpu_id][type];

    lock(&pm_global_lk);
    if (page_full(object_cache->slab_free))
    {
        Log("Get new page");
        object_cache->slab_free = (page_header *)((uint8_t *)buddy_alloc(PAGE_SIZE) + PAGE_SIZE - sizeof(page_header));
        object_cache->slab_free->parent_cpu_id = cur_cpu_id;
        Log("object_cache->slab_free:%p", object_cache->slab_free);
    }
    unlock(&pm_global_lk);

    lock(&page_lk);
    size_t i = 0, j = 0;
    Log("object_cache->slab_free->bitmap[0]:%u", object_cache->slab_free->bitmap[0]);
    while (object_cache->slab_free->bitmap[i] + 1ULL == 0ULL)
        i++;
    while (object_cache->slab_free->bitmap[i] & (1 << j))
        j++;
    object_cache->slab_free->bitmap[i] |= (1 << j);
    unlock(&page_lk);

    ret = (void *)((uint8_t *)object_cache->slab_free - (PAGE_SIZE - sizeof(page_header)) + (i * 64 + j) * size);
    return ret;
}

static void *kalloc(size_t size)
{
    void *ret = NULL;
    Log("kalloc: %d", size);
    size = binalign(size);
    Log("kalloc aligned size: %d", size);
    if (size >= PAGE_SIZE)
        ret = buddy_alloc(size);
    else
        ret = slab_alloc(size);
    return ret;
}

static void kfree(void *ptr)
{
    uintptr_t page_addr = (uintptr_t)ptr / PAGE_SIZE * PAGE_SIZE;
    page_header *cur = (page_header *)(page_addr + PAGE_SIZE - sizeof(page_header));

    uint64_t items = ((uintptr_t)ptr - page_addr) / (1 << cur->slab_type);
    lock(&page_lk);
    cur->bitmap[items / 64] ^= (1 << (items % 64));
    unlock(&page_lk);
}

static void pmm_init()
{
    cpu_num = cpu_count();
    assert(cpu_num <= MAX_CPU_NUM);

    buddy_init();
    Log("pmm_init finished");
}

MODULE_DEF(pmm) = {
    .init = pmm_init,
    .alloc = kalloc,
    .free = kfree,
};
