#include <pmm.h>

Cache cache[MAX_CPU_NUM][MAX_SLAB_TYPE];
Cache page[MAX_CPU_NUM];
size_t slab_type[MAX_SLAB_TYPE + 1] = {8, 16, 32, 64, 128, 256, 512, 1024, 2048};

void cache_init(void *start, size_t size, uint8_t type)
{
    size_t unit_size = slab_type[type];
    size_t unit_max_num = size / unit_size - 1;
    *((page_header *)start) = (page_header){
        .prev = NULL,
        .next = (type == MAX_SLAB_TYPE ? start + size : NULL),
        .entry = start + size,
        .cpu = CPU_CUR,
        .type = type,
        .units_remaining = unit_max_num,
    };

    void *pm_cur = start + unit_size;
    for (int i = 1; i < unit_max_num; i++)
    {
        *((void **)pm_cur) = pm_cur + unit_size;
        pm_cur += unit_size;
    }
    *((void **)pm_cur) = NULL;
}

static void *slab_get_page()
{
    page_header *ret = NULL;
    lock(&page[CPU_CUR].lk);
    if (page[CPU_CUR].entry == NULL)
    {
        unlock(&page[CPU_CUR].lk);
        return buddy_alloc(PAGE_SIZE);
    }
    ret = page[CPU_CUR].entry;
    page[CPU_CUR].entry = ret->next;
    unlock(&page[CPU_CUR].lk);
    return ret;
}

void *slab_alloc(size_t size)
{
    void *ret = NULL;
    int i = 0;
    while (slab_type[i] < size)
        i++;

    lock(&cache[CPU_CUR][i].lk);
    if (cache[CPU_CUR][i].entry == NULL)
    {
        cache[CPU_CUR][i].entry = slab_get_page();
        cache_init(cache[CPU_CUR][i].entry, PAGE_SIZE, i);
    }

    page_header *cur_page = cache[CPU_CUR][i].entry;

    ret = cur_page->entry;
    cur_page->entry = *(void **)ret;
    cur_page->units_remaining -= 1;
    if (cur_page->units_remaining == 0)
    {
        if (cur_page->next != NULL)
            ((page_header *)cur_page->next)->prev = NULL;
        cache[cur_page][i].entry = cur_page->next;
    }
    unlock(&cache[CPU_CUR][i].lk);
    return ret;
}

void slab_init(int cpu, void *start, size_t size)
{
    Log("slab system starts from %p to %p", start, end);

    for (int i = 0; i < MAX_SLAB_TYPE; i++)
    {
        cache[cpu][i] = (Cache){
            .entry = NULL,
            .lk = 0,
        };
    }

    page[cpu] = (Cache){
        .entry = start,
        .lk = 0,
    };

    cache_init(start, size, MAX_SLAB_TYPE);
    Log("slab_init finished");
}