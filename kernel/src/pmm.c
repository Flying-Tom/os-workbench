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

/* bitmap */
#define BITMAP_FULL(page, id) (page->bitmap[id] + 1ULL == 0ULL)

/*------------------------------------------*/
static lock_t pm_global_lk = LOCK_INIT();
static lock_t page_lk = LOCK_INIT();

static uintptr_t pm_start, pm_end;
static uint8_t cpu_id, cpu_num;
static size_t total_page_num;
static uint8_t max_order;
static size_t max_buddy_node_num;

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

enum
{
    BUD_UNINIT,
    BUD_EMPTY,
    BUD_USED
};

typedef struct buddy_node
{
    uint8_t status;
    uint8_t order;
} buddy_node;

buddy_node *buddy;

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
    //lock(&pm_global_lk);
    size_t ret = 0;
    if (free_list[order] == NULL)
        block_generate(order);
    ret = free_list[order]->id;
    free_list[order] = free_list[order]->next;
    //unlock(&pm_global_lk);
    return ret;
}
*/

static void buddy_init()
{
    Log("heap.start:%p", heap.start);
    Log("heap.end:%p", heap.end);
    pm_start = (uintptr_t)heap.start;
    pm_end = (uintptr_t)heap.end;

    pm_start = align(pm_start, PAGE_SIZE);

    max_order = log((pm_end - pm_start) / PAGE_SIZE) + 1;
    max_buddy_node_num = 1 << max_order;

    pm_end = (uintptr_t)((uint8_t *)pm_end - (max_buddy_node_num + 2) * sizeof(buddy_node));

    total_page_num = (pm_end - pm_start) / PAGE_SIZE;

    buddy = (buddy_node *)pm_end;

    int temp = total_page_num;
    //buddy_node *cur_node = NULL;
    for (int i = 0; i <= max_buddy_node_num; i++)
    {
        assert((uintptr_t)(buddy + i) < (uintptr_t)heap.end);
        buddy[i].status = BUD_UNINIT;
    }

    for (int i = max_order; i >= 2; i--)
    {
        int buddy_node_size = 1 << (max_order - 1);
        int j = 1 << (max_order - i);
        while (temp > buddy_node_size)
        {
            temp -= buddy_node_size;
            Log("buddy[%d] is empty now", j);
            buddy[j].status = BUD_EMPTY;
            j++;
        }
    }
    Log("max_buddy_node_num:%d", max_buddy_node_num);
    Log("total_page_num:%d", total_page_num);
    Log("max_order:%d", max_order);
    Log("pm_start:%p", pm_start);
    Log("pm_end:%p", pm_end);
}

static void *buddy_alloc(size_t size)
{
    /*
    lock(&pm_global_lk);
    void *ret = NULL;
    uint8_t order = 0;
    order = log(size / PAGE_SIZE) + 1;
    Log("buddy_alloc %d Bytes  Its order:%d", size, order);
    ret = (void *)PAGE(get_one_block(order));
    unlock(&pm_global_lk);
    return ret;
    */
    lock(&pm_global_lk);
    void *ret = NULL;
    unlock(&pm_global_lk);
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
    i = i << 8;
    while (tmp & 1)
    {
        tmp >>= 1;
        i++;
    }
    if (i + 1 < PAGE_SIZE / (1 << cur->slab_type))
        return false;
    else
        return true;
}

static void *slab_alloc(size_t size)
{
    void *ret = NULL;
    uint8_t type = 0, cur_cpu_id = 0;
    cur_cpu_id = cpu_current();

    type = cache_type(size);
    size = 1 << type;
    Log("type:%d size:%d", type, size);

    Cache *object_cache = &cache[cpu_id][type];
    if (page_full(object_cache->slab_free))
    {
        Log("Get new page");
        object_cache->slab_free = (page_header *)(buddy_alloc(PAGE_SIZE) + PAGE_SIZE - sizeof(page_header));
        object_cache->slab_free->parent_cpu_id = cur_cpu_id;
        Log("object_cache->slab_free:%p", object_cache->slab_free);
    }

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
    assert((uintptr_t)ret % size == 0);
    return ret;
}

static void *kalloc(size_t size)
{
    void *ret = NULL;
    Log("kalloc: %d", size);
    assert(size > 0);
    if (size >= PAGE_SIZE)
    {
        lock(&pm_global_lk);
        size = 1 << (log(size - 1) + 1);
        ret = buddy_alloc(size);
        Log("buddy_alloc size:%d ret:%p", size, ret);
        assert((uintptr_t)ret % size == 0);
        unlock(&pm_global_lk);
    }
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

    buddy_init();
    /*
    for (int i = 0; i < total_page_num; i++)
    {
        page_header *cur = PAGE_HEADER(i);
        cur->id = i;
        cur->parent_cpu_id = MAX_CPU_NUM;
        cur->next = NULL;
    }
    */

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
