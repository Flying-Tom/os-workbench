#include <common.h>
#include <lock.h>

#define BREAKPOINT(a) printf("BREAKPOINT:" #a "\n")
#define align(base, offset) (((base + offset - 1) / offset) * offset) // Right align
#define max(a, b) ((a > b) ? a : b)
#define PAGE_SIZE 4 KB
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

node_t local_nodelist[MAX_CPU_NUM];
node_t *global_nodelist;

static node_t *global_application(size_t size)
{
    return NULL;
}

static void *kalloc(size_t size)
{
    lock(&lk);
    cpu_id = cpu_current();
    node_t *cur = NULL, *cur_prev = NULL, *new_node = NULL;
    BREAKPOINT(kalloc);
    for (cur = local_nodelist[cpu_id].next; cur != NULL; cur_prev = cur, cur = cur->next)
    {
        if (cur->size >= size + sizeof(node_t))
        {
            new_node = (node_t *)(align(((uintptr_t)cur + sizeof(node_t) + cur->size - size), size) - sizeof(node_t));
            new_node->size = size;
            cur->size -= size + sizeof(node_t);

            void *ret = (void *)((uintptr_t)new_node + sizeof(node_t));
            //printf("ret:%p\n", ret);
            unlock(&lk);
            return ret;
        }
    }
    // local memory is insuffcient
    int pm_needed = max(size, pm_needed);
    cur_prev->next = global_application(pm_needed);

    unlock(&lk);
    return NULL;
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
    pm_start = align(pm_start, PAGE_SIZE);
    pm_end = (uintptr_t)heap.end;
    BREAKPOINT(Fuck);
    global_nodelist = (node_t *)pm_start;
    BREAKPOINT(Fuck again);
    global_nodelist->next = NULL;
    global_nodelist->size = pm_end - pm_start - sizeof(node_t);
    BREAKPOINT(Finsh Init);
}

MODULE_DEF(pmm) = {
    .init = pmm_init,
    .alloc = kalloc,
    .free = kfree,
};
