#include <pmm.h>

static int cache_lk[MAX_CPU_NUM], page_lk[MAX_CPU_NUM];
static void *cache_entry[MAX_CPU_NUM][MAX_SLAB_TYPE], *page_entry[MAX_CPU_NUM];
static size_t slab_type[MAX_SLAB_TYPE + 1] = {32, 64, 128, 256, 512, 1024, 2048, 4096};

static page_header *free_pagelist[MAX_CPU_NUM];

static inline void cache_init(void *start, size_t size, uint8_t type);

static inline void slab_page_free(void *ptr, uint8_t cpu)
{
    if (ptr < slab_start || ptr >= slab_end)
    {
        buddy_free(ptr);
        return;
    }

    lock(&page_lk[cpu]);
    *(void **)ptr = page_entry[cpu];
    page_entry[cpu] = ptr;
    unlock(&page_lk[cpu]);
}

void *slab_alloc(uint8_t order)
{
    Log("slab alloc %d bytes", 1 << order);
    void *ret = NULL;

    assert(order < PAGE_ORDER);
    uint8_t i = 0;
    i = max(0, order - 5);

    Log("Slab type:%d", slab_type[i]);

    page_header *cur_page = NULL;
    lock(&cache_lk[CPU_CUR]);
    if (cache_entry[CPU_CUR][i] == NULL)
    {

        if (free_pagelist[CPU_CUR] == NULL)
        {
            lock(&page_lk[CPU_CUR]);
            void *temp = buddy_alloc(PAGE_ORDER);
            unlock(&page_lk[CPU_CUR]);

            if (temp == NULL)
            {
                unlock(&cache_lk[CPU_CUR]);
                return NULL;
            }

            cur_page = (page_header *)temp;
            cur_page->next = NULL;
            free_pagelist[CPU_CUR] = temp;
        }
        cache_entry[CPU_CUR][i] = free_pagelist[CPU_CUR];
        cur_page = (page_header *)free_pagelist[CPU_CUR];
        free_pagelist[CPU_CUR] = cur_page->next;
        cache_init(cache_entry[CPU_CUR][i], PAGE_SIZE, i);
    }

    cur_page = cache_entry[CPU_CUR][i];

    ret = cur_page->entry;
    cur_page->units_remaining--;
    if (cur_page->units_remaining == 0)
    {
        cache_entry[CPU_CUR][i] = cur_page->next;
        cur_page->next = NULL;
        cur_page->prev = NULL;
    }
    cur_page->entry = *(void **)ret;
    unlock(&cache_lk[CPU_CUR]);
    return ret;
}

void slab_free(void *ptr)
{
    if ((uintptr_t)ptr & PAGE_RMASK)
    {
        page_header *cur_page = (void *)((uintptr_t)ptr & PAGE_LMASK);
        lock(&cache_lk[cur_page->cpu]);

        cur_page->units_remaining++;
        *(void **)ptr = cur_page->entry;
        cur_page->entry = ptr;

        if (cur_page->units_remaining == 1)
        {
            page_header *tar_page = (page_header *)cache_entry[cur_page->cpu][cur_page->type];

            if (tar_page == NULL)
            {
                cache_entry[cur_page->cpu][cur_page->type] = (void *)cur_page;
            }
            else
            {
                cur_page->next = tar_page;
                tar_page->prev = cur_page;
                cache_entry[cur_page->cpu][cur_page->type] = (void *)cur_page;
            }
            return;
        }
        else if (cur_page->units_remaining + 1 == ((PAGE_SIZE) / slab_type[cur_page->type]))
        {
            if (cache_entry[cur_page->cpu][cur_page->type] == (void *)cur_page)
                return;
            else
            {
                if (cur_page->next)
                    cur_page->next->prev = cur_page->prev;
                if (cur_page->prev)
                    cur_page->prev->next = cur_page->next;
                cur_page->next = free_pagelist[cur_page->cpu];
                cur_page->prev = NULL;
                free_pagelist[cur_page->cpu] = (void *)cur_page;
            }
            return;
        }
        unlock(&cache_lk[cur_page->cpu]);
    }

}

static inline void cache_init(void *start, size_t size, uint8_t type)
{
    size_t unit_size = slab_type[type];
    size_t unit_max_num = size / unit_size - 1;
    *((page_header *)start) = (page_header){
        .prev = NULL,
        .next = (type == MAX_SLAB_TYPE ? start + unit_size : NULL),
        .entry = start + unit_size,
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

void slab_init(uint8_t cpu, void *start, size_t size)
{
    Log("slab system starts from %p", start);

    for (int i = 0; i < MAX_SLAB_TYPE; i++)
    {
        cache_lk[cpu] = 0;
        cache_entry[cpu][i] = NULL;
    }

    page_lk[cpu] = 0;
    page_entry[cpu] = start;

    cache_init(start, size, MAX_SLAB_TYPE);
    Log("slab_init finished");
}