#include <common.h>
#include <lock.h>

#define BREAKPOINT(a) Log("BREAKPOINT:" #a "\n")
#define align(base, offset) (((base + offset - 1) / offset) * offset) // Right align
#define max(a, b) ((a > b) ? (a) : (b))
#define PAGE_SIZE (8 KB)
#define MAX_CPU_NUM 8
#define PAGE_HEADER(a) (page_header *)(pm_start + (a + 1) * PAGE_SIZE - sizeof(page_header))
#define PAGE(a) (pm_start + a * PAGE_SIZE)

static lock_t lk = LOCK_INIT();

/////////////////////////////
static uintptr_t pm_start, pm_end;
static int cpu_id, cpu_num;

typedef struct node_t
{
    int size;
    struct node_t *next;
} node_t;

typedef struct page_header
{
    int parent_cpu_id;
    int size;
    int inode_num;
    int slab_type;
    struct page_header *next;
} page_header;
page_header *global_page_list, *global_last_page;
int global_page_cnt;
page_header *slab_list[MAX_CPU_NUM][7];

node_t local_nodelist[MAX_CPU_NUM];
node_t *global_nodelist;

static node_t *__attribute__((used)) global_application(size_t size)
{
    if (global_nodelist->size > size + sizeof(node_t))
    {
        global_nodelist->size -= size + sizeof(node_t);
        Log("global_nodelist free size:%d\n", global_nodelist->size);
        return (node_t *)((uintptr_t)global_nodelist + global_nodelist->size - size);
    }
    Log("Kalloc Failed!\n");
    return NULL;
}

static int poweraligned(int x)
{
    int ret = 1;
    while (ret < x)
        ret = ret << 1;
    return ret;
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
            //Log("return page %d\n", i);
            return cur;
        }
    }
    BREAKPOINT(No more free pages);
    return NULL;
}

static void *slab_alloc(size_t size)
{
    //BREAKPOINT(slab_alloc);
    void *ret = NULL;
    int slab_type = 0;
    if (size > 0 && size <= 4)
        slab_type = 0;
    else if (size > 4 && size <= 8)
        slab_type = 1;
    else if (size > 8 && size <= 16)
        slab_type = 2;
    else if (size > 16 && size <= 32)
        slab_type = 3;
    else if (size > 32 && size <= 64)
        slab_type = 4;
    else if (size > 64 && size <= 128)
        slab_type = 5;

    //Log("slab_type:%d\n", slab_type);
    page_header *object_slab_list = slab_list[cpu_id][slab_type];
    if (object_slab_list == NULL || object_slab_list->size <= size)
    {
        object_slab_list = slab_list[cpu_id][slab_type] = get_one_page(size);
    }
    ret = (void *)((uintptr_t *)object_slab_list - (PAGE_SIZE - sizeof(page_header)) + (1 << (slab_type + 2)) * object_slab_list->inode_num);
    object_slab_list->inode_num++;

    object_slab_list->size -= (1 << (slab_type + 2));
    //assert((uintptr_t)ret % (1 << size) == 0);
    return ret;
}

static void *buddy_alloc(size_t size)
{
    cpu_id = cpu_current();
    size = poweraligned(size);
    Log("poweraligned(size):%d", poweraligned(size));
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
            return (void *)PAGE(i);
        }
    }
    /*
    cpu_id = cpu_current();
    node_t *cur = NULL, *cur_prev = &local_nodelist[cpu_id], *new_node = NULL;
    BREAKPOINT(kalloc);
    for (cur = &local_nodelist[cpu_id]; cur != NULL; cur_prev = cur, cur = cur->next)
    {
        if (cur->size >= size + sizeof(node_t))
        {
            new_node = (node_t *)(align(((uintptr_t)cur + sizeof(node_t) + cur->size - size), size) - sizeof(node_t));
            new_node->size = size;
            cur->size -= size + sizeof(node_t);
            //Log("cur->size:%d\n", cur->size);
            void *ret = (void *)((uintptr_t)new_node + sizeof(node_t));
            //Log("ret:%p\n", ret);
            unlock(&lk);
            return ret;
        }
    }
    BREAKPOINT(local memory is insuffcient);
    int pm_needed = 0;
    pm_needed = max(size, pm_needed);
    cur_prev->next = global_application(pm_needed);
    */
    return NULL;
}

static void *kalloc(size_t size)
{
    //BREAKPOINT(kalloc);
    void *ret = NULL;
    if (size == 0)
        return NULL;
    if (size > 128)
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
        cur->size = PAGE_SIZE - sizeof(page_header);
    }
    //assert(0);
    Log("pmm_init finished");
}

MODULE_DEF(pmm) = {
    .init = pmm_init,
    .alloc = kalloc,
    .free = kfree,
};
