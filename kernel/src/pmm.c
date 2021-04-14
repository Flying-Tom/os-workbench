#include <common.h>
#include <lock.h>

#define BREAKPOINT(a) Log("BREAKPOINT:" #a "\n")
#define align(base, offset) (((base + offset - 1) / offset) * offset) // Right align

/* const macro */
#define PAGE_SIZE (8 KB)
#define MAX_CPU_NUM 8
#define MAX_SLAB_TYPE 12

/* page header */
#define PAGE_HEADER(a) (page_header *)(pm_start + (a + 1) * PAGE_SIZE - sizeof(page_header))
#define PAGE(a) (pm_start + a * PAGE_SIZE)

/*------------------------------------------*/
static lock_t lk = LOCK_INIT();
static uintptr_t pm_start, pm_end;
static uint8_t cpu_id, cpu_num;
static size_t total_page_num;

typedef struct page_header
{
    size_t size;
    uint8_t parent_cpu_id;
    uint8_t slab_type;
    struct page_header *next;
} page_header;
page_header *global_page_list, *global_last_page;

typedef struct Cache
{
    int size;
    int num_of_slab;
    page_header *newest_slab;
} Cache;
Cache cache[MAX_CPU_NUM][8]

    static size_t
    log(size_t x)
{
    size_t ret = 0;
    while (x > 1)
    {
        x >>= 1;
        ret++;
    }
    return ret;
}

static uint8_t cache_type(size_t size)
{
    uint8_t slab_type = 0;
    slab_type = log(size - 1);
    return slab_type;
}

static page_header *get_one_page(size_t size)
{
    for (int i = 0; i < total_page_num; i++)
    {
        page_header *cur = PAGE_HEADER(i);
        if (cur->parent_cpu_id == MAX_CPU_NUM)
        {
            cur->parent_cpu_id = cpu_id;
            //cur->size = 0;
            Log("return page %d\n", i);
            return cur;
        }
    }
    Log("No more free pages");
    return NULL;
}

static void *slab_alloc(size_t size)
{
    void *ret = NULL;
    uint8_t type = 0;
    type = cache_type(size);
    size = 1 << type;
    Log("type:%d size:%d", type, size);
    Cache *object_cache = &cache[cpu_id][type];
    if (object_cache->newest_slab == NULL || object_cache->newest_slab->size + size >= PAGE_SIZE - sizeof(page_header))
    {
        Log("Get new page");
        object_cache->newest_slab = get_one_page(size);
    }

    ret = (void *)((uint8_t *)object_cache->newest_slab - (PAGE_SIZE - sizeof(page_header)) + object_cache->newest_slab->size);
    object_cache->newest_slab->size += size;

    assert((uintptr_t)ret % size == 0);
    return ret;
}

static void *buddy_alloc(size_t size)
{
    lock(&lk);
    cpu_id = cpu_current();
    for (int i = 0; i < total_page_num; i++)
    {
        page_header *cur = PAGE_HEADER(i);

        int page_needed = (size - 1) / PAGE_SIZE + 1;
        for (int j = i; j <= i + page_needed; j++)
        {
            cur = PAGE_HEADER(j);
            cur->parent_cpu_id = cpu_id;
        }
        unlock(&lk);
        return (void *)PAGE(i);
    }
    return NULL;
}

static void *kalloc(size_t size)
{
    void *ret = NULL;
    assert(size > 0);
    if (size > PAGE_SIZE)
    {
        lock(&lk);
        ret = buddy_alloc(size);
        unlock(&lk);
    }
    else
        ret = slab_alloc(size);
    return ret;
}

static void kfree(void *ptr)
{
    lock(&lk);
    unlock(&lk);
}

static void pmm_init()
{
    cpu_num = cpu_count();
    assert(cpu_num <= MAX_CPU_NUM);

    pm_start = (uintptr_t)heap.start;
    pm_end = (uintptr_t)heap.end;
    Log("pm_start:%p aligned pm_start:%p", pm_start, align(pm_start, PAGE_SIZE));
    pm_start = align(pm_start, PAGE_SIZE);
    total_page_num = (pm_end - pm_start) / PAGE_SIZE;

    for (int i = 0; i < total_page_num; i++)
    {
        page_header *cur = PAGE_HEADER(i);
        cur->parent_cpu_id = MAX_CPU_NUM;
        cur->size = 0;
    }
    assert((pm_end - pm_start) % PAGE_SIZE == 0);
    Log("Total pages:%d", total_page_num);
    Log("pmm_init finished");
}

MODULE_DEF(pmm) = {
    .init = pmm_init,
    .alloc = kalloc,
    .free = kfree,
};
