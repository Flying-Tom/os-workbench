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
    uint8_t ret = 0;
    ret = max(3, log(size - 1) + 1);
    return ret;
}

static void block_generate(uint8_t order)
{
    //Log("%d",order);
    //assert(order < max_order);
    if (free_list[order + 1] == NULL)
    {
        //assert(0);
        block_generate(order + 1);
    }
    size_t parent_page_id = free_list[order + 1]->id;
    free_list[order + 1] = free_list[order + 1]->next;

    free_list[order] = PAGE_HEADER(parent_page_id + (1 << order));
    page_header *newpage = PAGE_HEADER(parent_page_id);
    newpage->next = free_list[order];
    free_list[order] = newpage;
}

static size_t get_one_block(uint8_t order)
{
    size_t ret = 0;
    if (free_list[order] == NULL)
        block_generate(order);
    ret = free_list[order]->id;
    free_list[order] = free_list[order]->next;
    return ret;
}

static void *buddy_alloc(size_t size)
{
    lock(&lk);
    void *ret = NULL;
    uint8_t order = 0;
    order = log(size / PAGE_SIZE) + 1;
    Log("buddy_alloc %d Bytes  Its order:%d", size, order);
    ret = (void *)PAGE(get_one_block(order));
    unlock(&lk);
    return ret;
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
        object_cache->newest_slab = (page_header *)((uint8_t *)buddy_alloc(PAGE_SIZE) + (PAGE_SIZE - sizeof(page_header)));
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
        //lock(&lk);
        size = 1 << (log(size - 1) + 1);
        ret = buddy_alloc(size);
        Log("buddy_alloc size:%d ret:%p", size, ret);
        //assert((uintptr_t)ret % size == 0);
        //unlock(&lk);
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

void buddy_stat()
{
    printf("=========================\n");
    for (int i = max_order; i >= 1; i--)
    {
        if (free_list[i] != NULL)
        {
            page_header *cur = free_list[i];
            while (cur != NULL)
            {
                printf("order %d block:%d  ", i, cur->id);
                cur = cur->next;
            }
            printf("\n");
        }
    }
    printf("=========================\n");
    assert(0);
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

    for (int i = 0; i < total_page_num; i++)
    {
        page_header *cur = PAGE_HEADER(i);
        cur->id = i;
        cur->parent_cpu_id = MAX_CPU_NUM;
        cur->size = 0;
        cur->next = NULL;
    }

    free_list[max_order] = PAGE_HEADER(0);

    get_one_block(13);

    buddy_stat();
    assert((pm_end - pm_start) % PAGE_SIZE == 0);
    Log("Total pages:%d", total_page_num);
    Log("pmm_init finished");
}

MODULE_DEF(pmm) = {
    .init = pmm_init,
    .alloc = kalloc,
    .free = kfree,
};
