#include <common.h>
#include <lock.h>

#define BREAKPOINT(a) printf("BREAKPOINT:" #a "\n")
#define align(base, offset) (((base + offset - 1) / offset) * offset) // Right align
#define max(a, b) ((a > b) ? (a) : (b))
#define PAGE_SIZE (4 KB)
#define MAX_CPU_NUM 8

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
        printf("global_nodelist free size:%d\n", global_nodelist->size);
        return (node_t *)((uintptr_t)global_nodelist + global_nodelist->size - size);
    }
    printf("Kalloc Failed!\n");
    return NULL;
}

static page_header *get_one_page()
{
    BREAKPOINT(get_one_page);
    page_header *next_page = (page_header *)((uintptr_t)(pm_start + global_page_cnt * PAGE_SIZE) + sizeof(page_header));
    next_page->parent_cpu_id = cpu_id;
    next_page->size = PAGE_SIZE;
    return next_page;
}

static void *slab_alloc(size_t size)
{
    BREAKPOINT(slab_alloc);
    void *ret = NULL;
    int slab_type = size / 4;
    page_header *object_slab_list = slab_list[cpu_id][slab_type];
    if (object_slab_list == NULL || object_slab_list->size <= size)
        object_slab_list = get_one_page();

    ret = (void *)((uintptr_t *)object_slab_list + (slab_type + 1) * 4 * object_slab_list->inode_num);
    object_slab_list->inode_num++;
    object_slab_list->size -= (slab_type + 1) * 4;

    return ret;
}

static void *buddy_alloc(size_t size)
{ /*
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
            //printf("cur->size:%d\n", cur->size);
            void *ret = (void *)((uintptr_t)new_node + sizeof(node_t));
            //printf("ret:%p\n", ret);
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
    BREAKPOINT(kalloc);
    void *ret = NULL;
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

/*
static void pmm_stat()
{
    node_t *cur;
    int node_cnt = 0;
    printf("============================\n");
    for (cur = root_node; cur != NULL; cur = cur->next)
    {
        printf("Node %d | status:%d  size:%d MB\n", node_cnt++, cur->status, cur->size / (1024 * 1024));
    }
    printf("============================\n");
}
*/

static void pmm_init()
{
    /*
    uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
    printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);
    */
    cpu_num = cpu_count();
    assert(cpu_num <= MAX_CPU_NUM);

    pm_start = (uintptr_t)heap.start;
    printf("pm_start:%p\n", pm_start);
    pm_start = align(pm_start, PAGE_SIZE);
    printf("aligned pm_start:%p\n", pm_start);
    printf("Total pages:%d\n", (pm_end - pm_start) / PAGE_SIZE);
    assert((pm_end - pm_start) % PAGE_SIZE == 0);
    pm_end = (uintptr_t)heap.end;
    global_nodelist = (node_t *)pm_start;
    global_nodelist->next = NULL;
    global_nodelist->size = pm_end - pm_start - sizeof(node_t);
    assert(0);
}

MODULE_DEF(pmm) = {
    .init = pmm_init,
    .alloc = kalloc,
    .free = kfree,
};
