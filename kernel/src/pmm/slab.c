#include <pmm.h>

int cache_lk[MAX_CPU_NUM][MAX_SLAB_TYPE], page_lk[MAX_CPU_NUM];
void *cache_entry[MAX_CPU_NUM][MAX_SLAB_TYPE], *page_entry[MAX_CPU_NUM];

size_t slab_type[MAX_SLAB_TYPE + 1] = {8, 16, 32, 64, 128, 256, 512, 1024, 4096};

static void cache_init(void *start, size_t size, uint8_t type)
{
    size_t unit_size = slab_type[type];
    size_t unit_max_num = size / unit_size - 1;
    *((page_header *)start) = (page_header){
        .prev = NULL,
        .next = (type == MAX_SLAB_TYPE ? start + unit_size : NULL),
        .entry = start + unit_size,
        .cpu = (uint8_t)CPU_CUR,
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
    lock(&page_lk[CPU_CUR]);
    if (page_entry[CPU_CUR] == NULL)
    {
        unlock(&page_lk[CPU_CUR]);
        return buddy_alloc(PAGE_SIZE);
    }
    page_header *ret = page_entry[CPU_CUR];
    page_entry[CPU_CUR] = ret->next;
    unlock(&page_lk[CPU_CUR]);
    return ret;
}

void *slab_alloc(size_t size)
{
    Log("slab alloc %d bytes", size);
    void *ret = NULL;
    if (size > slab_type[MAX_SLAB_TYPE - 1])
        return slab_get_page();
    else
    {
        uint8_t i = 0;
        while (slab_type[i] < size)
            i++;

        Log("Slab type:%d", slab_type[i]);

        lock(&cache_lk[CPU_CUR][i]);
        if (cache_entry[CPU_CUR][i] == NULL)
        {
            cache_entry[CPU_CUR][i] = slab_get_page();
            if (cache_entry[CPU_CUR][i] == NULL)
            {
                unlock(&cache_lk[CPU_CUR][i]);
                return NULL;
            }
            cache_init(cache_entry[CPU_CUR][i], PAGE_SIZE, i);
        }

        page_header *cur_page = cache_entry[CPU_CUR][i];

        ret = cur_page->entry;
        cur_page->entry = *(void **)ret;
        cur_page->units_remaining -= 1;
        if (cur_page->units_remaining == 0)
        {
            if (cur_page->next != NULL)
                ((page_header *)cur_page->next)->prev = NULL;
            cache_entry[CPU_CUR][i] = cur_page->next;
        }
        unlock(&cache_lk[CPU_CUR][i]);
    }
    return ret;
}

void slab_init(int cpu, void *start, size_t size)
{
    Log("slab system starts from %p", start);

    for (int i = 0; i < MAX_SLAB_TYPE; i++)
    {
        cache_lk[cpu][i] = 0;
        cache_entry[cpu][i] = NULL;
    }

    page_lk[cpu] = 0;
    page_entry[cpu] = start;

    cache_init(start, size, MAX_SLAB_TYPE);
    Log("slab_init finished");
}