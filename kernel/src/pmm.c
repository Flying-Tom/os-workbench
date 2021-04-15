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
#define slab_max_items(size) ((PAGE_SIZE - sizeof(page_header)) / size)

/* buddy system*/
#define MAX_BUDDY_BLOCK_TYPE 20

/*------------------------------------------*/
static lock_t lk = LOCK_INIT();
static uintptr_t pm_start, pm_end, pm_cur;
static uint8_t cpu_id, cpu_num;
static size_t total_page_num;
static uint8_t max_order;

typedef struct page_header
{
    size_t id;
    uint8_t parent_cpu_id;
    uint64_t bitmap[4096];
    uint8_t slab_type;
    uint8_t order;
    struct page_header *next;
} page_header;
page_header *free_list[MAX_BUDDY_BLOCK_TYPE];

typedef struct Cache
{
    int size;
    int num_of_slab;
    page_header *slab_free;
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

/*
static void block_generate(uint8_t order)
{
    assert(order < max_order);
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
    //lock(&lk);
    size_t ret = 0;
    if (free_list[order] == NULL)
        block_generate(order);
    ret = free_list[order]->id;
    free_list[order] = free_list[order]->next;
    //unlock(&lk);
    return ret;
}
*/

static void *buddy_alloc(size_t size)
{
    /*
    lock(&lk);
    void *ret = NULL;
    uint8_t order = 0;
    order = log(size / PAGE_SIZE) + 1;
    Log("buddy_alloc %d Bytes  Its order:%d", size, order);
    ret = (void *)PAGE(get_one_block(order));
    unlock(&lk);
    return ret;
    */
    void *ret = NULL;
    pm_cur -= size;
    return ret;
}

static bool page_full(page_header *cur)
{
    if (cur == NULL)
        return true;
    uint64_t i, tmp = 0;
    for (i = 0; i < 64; i++)
    {
        if (cur->bitmap[i] + 1ULL != 0ULL)
        {
            tmp = cur->bitmap[i];
            break;
        }
    }
    i = i * 64;
    while (tmp & (1ULL))
    {
        tmp >>= 1ULL;
        i++;
    }
    if (i + 1 < PAGE_SIZE / (1 << cur->slab_type))
        return 0;
    else
        return 1;
}

static void *slab_alloc(size_t size)
{
    void *ret = NULL;
    uint8_t type = 0;

    type = cache_type(size);
    size = 1 << type;
    Log("type:%d size:%d", type, size);

    Cache *object_cache = &cache[cpu_id][type];
    if (page_full(object_cache->slab_free))
    {
        Log("Get new page");
        object_cache->slab_free = (page_header *)(buddy_alloc(size) + PAGE_SIZE - sizeof(page_header));
    }
    size_t i = 0, j = 0;
    while (object_cache->slab_free->bitmap[i])
        i++;
    while (object_cache->slab_free->bitmap[i] & (1 << j))
        j++;
    object_cache->slab_free->bitmap[i] |= (1 << j);

    ret = (void *)((uint8_t *)object_cache->slab_free + (i * 64 + j) * size);
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
    pm_cur = pm_end;

    total_page_num = (pm_end - pm_start) / PAGE_SIZE;
    Log("total_page_num:%d", total_page_num);
    max_order = log(total_page_num);
    Log("max_order:%d", max_order);

    for (int i = 0; i < total_page_num; i++)
    {
        page_header *cur = PAGE_HEADER(i);
        cur->id = i;
        cur->parent_cpu_id = MAX_CPU_NUM;
        cur->next = NULL;
    }

    //free_list[max_order] = PAGE_HEADER(0);

    //buddy_stat();
    assert((pm_end - pm_start) % PAGE_SIZE == 0);
    Log("Total pages:%d", total_page_num);
    Log("pmm_init finished");
}

MODULE_DEF(pmm) = {
    .init = pmm_init,
    .alloc = kalloc,
    .free = kfree,
};
