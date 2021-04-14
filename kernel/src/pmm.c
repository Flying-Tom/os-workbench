#include <common.h>
#include <lock.h>

#define BREAKPOINT(a) Log("BREAKPOINT:" #a "\n")
#define align(base, offset) (((base + offset - 1) / offset) * offset) // Right align

/* const macro */
#define PAGE_SIZE (4 KB)
#define MAX_CPU_NUM 8
#define MAX_SLAB_TYPE 12

/* page header */
#define PAGE_HEADER(a) (page_header *)(pm_start + (a + 1) * PAGE_SIZE - sizeof(page_header))
#define PAGE(a) (pm_start + a * PAGE_SIZE)

/* buddy system*/
#define MAX_BUDDY_BLOCK_TYPE 20

/*------------------------------------------*/
static lock_t lk = LOCK_INIT();
static uintptr_t pm_start, pm_end;
static uint8_t cpu_id, cpu_num;
static size_t total_page_num;
static uint8_t max_order;

typedef struct page_header
{
    size_t size;
    size_t id;
    uint8_t parent_cpu_id;
    uint8_t slab_type;
    uint8_t order;
    struct page_header *next;
} page_header;
page_header *free_list[MAX_BUDDY_BLOCK_TYPE];

typedef struct Cache
{
    int size;
    int num_of_slab;
    page_header *newest_slab;
} Cache;
Cache cache[MAX_CPU_NUM][8];

static size_t log(size_t x)
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

static void get_one_block(uint8_t order)
{
    if (free_list[order + 1] == NULL)
        get_one_block(order + 1);
    free_list[order] = PAGE_HEADER(free_list[order + 1]->id + (1 << order));
    page_header *newpage = PAGE_HEADER(free_list[order + 1]->id);
    newpage->next = free_list[order];
    free_list[order] = newpage;

    free_list[order + 1] = free_list[order + 1]->next;
    //assert(free_list[order + 1]->next != NULL);
    if (free_list[order + 1]->next != NULL)
        Log("free_list[order + 1]->id:%d", free_list[order + 1]->id);
    Log("order:%d", order);
    //assert(0);
}

static void *buddy_alloc(size_t size)
{
    /*
    lock(&lk);
    cpu_id = cpu_current();
    for (int i = 0; i < total_page_num; i++)
    {
        page_header *cur = PAGE_HEADER(i);
        if (cur->parent_cpu_id == MAX_CPU_NUM)
        {
            int page_needed = (size - 1) / PAGE_SIZE + 1;
            for (int j = i; j < i + page_needed; j++)
            {
                cur = PAGE_HEADER(j);
                cur->parent_cpu_id = cpu_id;
            }
            unlock(&lk);
            return (void *)PAGE(i);
        }
    }
    return NULL;
    */
    return NULL;
}

static page_header *get_one_page()
{
    return (page_header *)((uint8_t *)buddy_alloc(PAGE_SIZE) + (PAGE_SIZE - sizeof(page_header)));
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
        object_cache->newest_slab = get_one_page();
    }

    ret = (void *)((uint8_t *)object_cache->newest_slab - (PAGE_SIZE - sizeof(page_header)) + object_cache->newest_slab->size);
    object_cache->newest_slab->size += size;
    assert((uintptr_t)ret % size == 0);
    return ret;
}

static void *kalloc(size_t size)
{
    void *ret = NULL;
    assert(size > 0);
    if (size >= PAGE_SIZE)
    {
        lock(&lk);
        ret = buddy_alloc(size);
        assert((uintptr_t)ret % size == 0);
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
    Log("pm_end:%p", pm_end);
    pm_start = align(pm_start, PAGE_SIZE);
    total_page_num = (pm_end - pm_start) / PAGE_SIZE;
    Log("total_page_num:%d", total_page_num);
    max_order = log(total_page_num);
    Log("max_order:%d", max_order);

    free_list[max_order] = PAGE_HEADER(0);
    get_one_block(14);

    for (int i = max_order; i >= 1; i--)
    {
        if (free_list[i] != NULL)
            printf("free_list[%d] id:%d \n", i, free_list[i]->id);
    }

    for (int i = 0; i < total_page_num; i++)
    {
        page_header *cur = PAGE_HEADER(i);
        cur->parent_cpu_id = MAX_CPU_NUM;
        cur->size = 0;
    }
    assert(0);
    assert((pm_end - pm_start) % PAGE_SIZE == 0);
    Log("Total pages:%d", total_page_num);
    Log("pmm_init finished");
}

MODULE_DEF(pmm) = {
    .init = pmm_init,
    .alloc = kalloc,
    .free = kfree,
};
