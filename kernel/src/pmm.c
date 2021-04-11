#include <common.h>
#include <lock.h>

#define BREAKPOINT(a) printf("BREAKPOINT:" #a "\n");
#define align(addr, size) addr = ((addr + size - 1) / size) * size // Right align

static lock_t lk = LOCK_INIT();

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

static void list_insert(node_t *x, node_t *y)
{
    if (x->next == NULL)
    {
        x->next = y;
        y->prev = x;
    }
    else
    {
        x->next->prev = y;
        y->next = x->next;
        x->next = y;
        y->prev = x;
    }
}

static void *kalloc(size_t size)
{
    lock(&lk);
    node_t *cur;
    printf("%d\n", cpu_current());
    for (cur = root_node; cur != NULL; cur = cur->next)
    {
        if (cur->status == NODE_FREE && cur->size >= size + sizeof(node_t))
        {
            node_t *new_node = (node_t *)((((uintptr_t)cur + sizeof(node_t) + cur->size - size) >> size) << size) - sizeof(node_t);
            new_node->size = size;
            new_node->status = NODE_USED;
            cur->size = cur->size - size - sizeof(node_t);

            list_insert(cur, new_node);

            void *ret = (void *)((uintptr_t)new_node + sizeof(node_t));
            //printf("ret:%p\n", ret);
            unlock(&lk);
            return ret;
        }
    }
    //printf("Fail to alloc\n");
    unlock(&lk);
    return NULL;
}

static void kfree(void *ptr)
{
    lock(&lk);
    node_t *cur = (node_t *)((uintptr_t)ptr - sizeof(node_t));
    node_t *Lfree_section = cur;
    cur->status = NODE_FREE;
    while (Lfree_section->prev != NULL && Lfree_section->prev->status == NODE_FREE)
        Lfree_section = Lfree_section->prev;

    cur = Lfree_section->next;
    for (; cur->status == NODE_FREE; cur = cur->next)
    {
        Lfree_section->size += sizeof(node_t) + cur->size;
    }
    Lfree_section->next = cur;
    cur->prev = Lfree_section;
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

#ifndef TEST
static void pmm_init()
{
    /*
    uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
    printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);
    */
    root_node = (node_t *)heap.start;
    root_node->size = ((uintptr_t)heap.end - (uintptr_t)heap.start) - sizeof(node_t);
    root_node->status = NODE_FREE;
    root_node->next = root_node->prev = NULL;
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
};
