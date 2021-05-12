#include <pmm.h>

//extern lock_t pm_global_lk;

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

void *slab_alloc(size_t size)
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
        object_cache->slab_free = (page_header *)((uint8_t *)global_alloc(PAGE_SIZE) + PAGE_SIZE - sizeof(page_header));
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