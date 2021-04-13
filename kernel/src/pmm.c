#include <common.h>
#include <lock.h>

#define BREAKPOINT(a) Log("BREAKPOINT:" #a "\n")
#define align(base, offset) (((base + offset - 1) / offset) * offset) // Right align
#define max(a, b) ((a > b) ? (a) : (b))
#define PAGE_SIZE (8 KB)
#define MAX_CPU_NUM 8
#define MAX_SLAB_TYPE 12
#define PAGE_HEADER(a) (page_header *)(pm_start + (a + 1) * PAGE_SIZE - sizeof(page_header))
#define PAGE(a) (pm_start + a * PAGE_SIZE)

static lock_t lk = LOCK_INIT();

/////////////////////////////
static uintptr_t pm_start, pm_end;
static int cpu_id, cpu_num;

typedef struct page_header
{
    int parent_cpu_id;
    int size;
    int slab_type;
    struct page_header *next;
} page_header;
page_header *global_page_list, *global_last_page;
int global_page_cnt;

typedef struct Cache
{
    int size;
    int num_of_slab;
    page_header *newest_slab;
} Cache;
Cache cache[MAX_CPU_NUM][8];
int slab_type_size[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};

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

static int cache_type(size_t size)
{
    int slab_type = 0;
    slab_type = log(size - 1);
    /*
    if (size > 0 && size <= 16)
        slab_type = 0;
    else if (size > 16 && size <= 32)
        slab_type = 1;
    else if (size > 32 && size <= 64)
        slab_type = 2;
    else if (size > 64 && size <= 128)
        slab_type = 3;
    else if (size > 128 && size <= 256)
        slab_type = 4;
    else if (size > 256 && size <= 512)
        slab_type = 5;
    if (size <= 3)
    */

    return slab_type;
}

static page_header *get_one_page(size_t size)
{
    //BREAKPOINT(get_one_page);
    for (int i = 0; i < (pm_end - pm_start) / PAGE_SIZE; i++)
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
    //BREAKPOINT(slab_alloc);
    void *ret = NULL;
    int type = 0;
    type = cache_type(size);
    size = 1 << type;
    Log("type:%d", type);
    Log("size:%d", size);
    Cache *object_cache = &cache[cpu_id][type];
    if (object_cache->newest_slab == NULL || object_cache->newest_slab->size + size >= PAGE_SIZE - sizeof(page_header))
    {
        Log("Get new page");
        object_cache->newest_slab = get_one_page(size);
    }

    ret = (void *)((uint8_t *)object_cache->newest_slab - (PAGE_SIZE - sizeof(page_header)) + object_cache->newest_slab->size);
    //Log("object_cache->newest_slab->size:%d", object_cache->newest_slab->size);
    object_cache->newest_slab->size += size;

    assert((uintptr_t)ret % size == 0);
    return ret;
}

static void *buddy_alloc(size_t size)
{
    lock(&lk);
    cpu_id = cpu_current();
    //size = poweraligned(size);
    //Log("poweraligned(size):%d", poweraligned(size));
    for (int i = 0; i < (pm_end - pm_start) / PAGE_SIZE; i++)
    {
        page_header *cur = PAGE_HEADER(i);
        if (PAGE(i) % size == 0)
        {
            int page_needed = size / PAGE_SIZE + 1;
            for (int j = i; j <= i + page_needed; j++)
            {
                cur = PAGE_HEADER(j);
                cur->parent_cpu_id = cpu_id;
            }
            unlock(&lk);
            return (void *)PAGE(i);
        }
    }
    return NULL;
}

static void *kalloc(size_t size)
{
    //BREAKPOINT(kalloc);
    void *ret = NULL;
    if (size == 0)
        return NULL;
    if (size > 512)
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

/*
static void pmm_stat()
{
    node_t *cur;
    int node_cnt = 0;
    Log("============================\n");
    for (cur = root_node; cur != NULL; cur = cur->next)
    {
        Log("Node %d | status:%d  size:%d MB\n", node_cnt++, cur->status, cur->size / (1024 * 1024));
    }
    Log("============================\n");
}
*/

static void pmm_init()
{
    /*
    uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
    Log("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);
    */
    cpu_num = cpu_count();
    assert(cpu_num <= MAX_CPU_NUM);

    pm_start = (uintptr_t)heap.start;
    pm_end = (uintptr_t)heap.end;
    //Log("pm_start:%p\n", pm_start);
    pm_start = align(pm_start, PAGE_SIZE);
    //Log("aligned pm_start:%p\n", pm_start);
    //Log("Total pages:%d\n", (pm_end - pm_start) / PAGE_SIZE);
    assert((pm_end - pm_start) % PAGE_SIZE == 0);

    for (int i = 0; i < (pm_end - pm_start) / PAGE_SIZE; i++)
    {
        page_header *cur = PAGE_HEADER(i);
        cur->parent_cpu_id = MAX_CPU_NUM;
        //cur->size = PAGE_SIZE - sizeof(page_header);
        cur->size = 0;
    }
    //assert(0);
    Log("pmm_init finished");
}

MODULE_DEF(pmm) = {
    .init = pmm_init,
    .alloc = kalloc,
    .free = kfree,
};
