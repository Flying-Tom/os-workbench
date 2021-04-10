#include <common.h>

#define BREAKPOINT(a) printf("BREAKPOINT:" #a "\n");

typedef struct
{
    int locked;
} lock_t;

#define LOCK_INIT() ((lock_t){.locked = 0})
void lock(lock_t *lk)
{
    while (atomic_xchg(&lk->locked, 1))
        ;
}
void unlock(lock_t *lk) { atomic_xchg(&lk->locked, 0); }

//static lock_t lk = LOCK_INIT();

/////////////////////////////

typedef struct node_t
{
    int size;
    int status;
    struct node_t *prev;
    struct node_t *next;
} node_t;

node_t *root_node;

enum
{
    NODE_FREE,
    NODE_USED
};

static void *kalloc(size_t size)
{
    //printf("kalloc\n");
    node_t *cur;
    for (cur = root_node; cur != NULL; cur = cur->next)
    {
        if (cur->status == NODE_FREE && cur->size >= size + sizeof(node_t))
        {
            node_t *new_node = (node_t *)((uintptr_t)cur + cur->size - size);
            new_node->size = size;
            new_node->status = NODE_USED;
            cur->size = cur->size - size - sizeof(node_t);
            new_node->next = cur->next;
            new_node->prev = cur;
            cur->next = new_node;
            cur->next->prev = new_node;
            printf("ret:%p\n", new_node);
            return new_node;
        }
    }
    printf("Fail to alloc\n");
    return NULL;
}

static void kfree(void *ptr)
{
    node_t *cur = (node_t *)ptr;
    node_t *Lfree_section = cur;
    cur->status = NODE_FREE;
    while (Lfree_section->prev->status == NODE_FREE)
        Lfree_section = Lfree_section->prev;
    BREAKPOINT(?)
    if (Lfree_section == cur)
        return;
    cur = Lfree_section->next;
    for (; cur->status == NODE_FREE; cur = cur->next)
    {
        Lfree_section->size += sizeof(node_t) + cur->size;
    }
    Lfree_section->next = cur;
    cur->prev = Lfree_section;
}

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

#ifndef TEST
static void pmm_init()
{
    uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
    root_node = (node_t *)heap.start;
    root_node->size = pmsize - sizeof(node_t);
    root_node->status = NODE_FREE;
    printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);
}
#else
static void pmm_init()
{
    char *ptr = malloc(HEAP_SIZE);
    heap.start = ptr;
    heap.end = ptr + HEAP_SIZE;
    printf("Got %d MiB heap: [%p, %p)\n", HEAP_SIZE >> 20, heap.start, heap.end);
}
#endif

MODULE_DEF(pmm) = {
    .init = pmm_init,
    .alloc = kalloc,
    .free = kfree,
    .stat = pmm_stat,
};
