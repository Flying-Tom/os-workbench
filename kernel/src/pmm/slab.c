#include <pmm.h>

lock_t pm_global_lk = LOCK_INIT();
lock_t cache_lk[MAX_CPU_NUM];

uintptr_t slab_start, slab_end, slab_cur;

static page_header *slab_get_page()
{
    page_header *ret = NULL;
    if (slab_cur + PAGE_SIZE < slab_end)
    {
        ret = (page_header *)(slab_cur + PAGE_SIZE - sizeof(page_header));
        slab_cur += PAGE_SIZE;
    }
    return ret;
}

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
    /*
    i = i << 8;
    while (tmp & 1)
    {
        tmp >>= 1;
        i++;
    }
    */

    return (i + 1 > (PAGE_SIZE - sizeof(page_header)) / (1 << cur->slab_type));
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
        object_cache->slab_free = slab_get_page();
        object_cache->slab_free->parent_cpu_id = cur_cpu_id;
    }
    unlock(&pm_global_lk);

    Log("object_cache->slab_free:%p", object_cache->slab_free);

    lock(&cache_lk[cur_cpu_id]);
    size_t i = 0, j = 0;
    Log("object_cache->slab_free->bitmap[0]:%u", object_cache->slab_free->bitmap[0]);
    while (object_cache->slab_free->bitmap[i] + 1ULL == 0ULL)
        i++;
    while (object_cache->slab_free->bitmap[i] & (1 << j))
        j++;
    object_cache->slab_free->bitmap[i] |= (1 << j);
    unlock(&cache_lk[cur_cpu_id]);

    ret = (void *)((uint8_t *)object_cache->slab_free - (PAGE_SIZE - sizeof(page_header)) + (i * 64 + j) * size);
    return ret;
}

void slab_init(uintptr_t start, uintptr_t end)
{
    Log("slab system starts from %p to %p", start, end);
    slab_cur = slab_start = start;
    slab_end = end;

    for (int i = 0; i < MAX_CPU_NUM; i++)
        cache_lk[i] = LOCK_INIT();

    total_page_num = (pm_end - pm_start) / PAGE_SIZE;
    Log("total_page_num:%d", total_page_num);
    max_order = log(total_page_num);
    Log("max_order:%d", max_order);
    Log("slab_init finished");
}