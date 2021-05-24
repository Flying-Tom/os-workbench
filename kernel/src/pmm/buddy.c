#include <pmm.h>

#define MAX_BUD_ORDER 24
#define MAX_BUD_SIZE (1 << 24)

lock_t buddy_lk = LOCK_INIT();
uint8_t buddy_root_order = MAX_BUD_ORDER;
uintptr_t buddy_start, buddy_end;

static void *buddy_alloc_search(int id, uint8_t cur_order, uint8_t tar_order)
{
    void *ret = NULL;
    if (cur_order < PAGE_ORDER || buddy[id].status == BUD_FULL || tar_order > buddy[id].order)
        return NULL;

    if ((ret = buddy_alloc_search(id * 2, cur_order - 1, tar_order)) == NULL)
        ret = buddy_alloc_search(id * 2 + 1, cur_order - 1, tar_order);

    if (ret != NULL)
    {
        buddy[id].order = max(buddy[id * 2].order, buddy[id * 2 + 1].order);
        buddy[id].status = BUD_SPLITTED;
        return ret;
    }

    if (buddy[id].status == BUD_EMPTY)
    {
        buddy[id].status = BUD_FULL;
        buddy[id].order = 0;
        return buddy[id].addr;
    }
    return NULL;
}

void *buddy_alloc(size_t size)
{
    void *ret = NULL;
    lock(&buddy_lk);
    ret = buddy_alloc_search(1, buddy_root_order, log(size));
    unlock(&buddy_lk);
    return ret;
}

static void budnode_init(int id, uint8_t order, void *ptr)
{
    if (order < PAGE_ORDER)
        return;
    buddy[id] = (buddy_node){
        .addr = ptr,
        .status = BUD_EMPTY,
        .order = order,
    };
    budnode_init(id * 2, order - 1, ptr);
    budnode_init(id * 2 + 1, order - 1, ptr + (1 << (order - 1)));
}
void buddy_init(uintptr_t start, uintptr_t end)
{
    Log("buddy system starts from %p to %p", start, end);
    size_t budnode_area_size = ((uintptr_t)(end - start) >> (MAX_BUD_ORDER - PAGE_ORDER)) * sizeof(buddy_node);
    buddy = start;
    buddy_start = align(((uintptr_t)budnode_area_start + budnode_area_size), MAX_BUD_SIZE);
    buddy_end = end;
    Log("buddy system really used space: %p -> %p", buddy_start, buddy_end);
    budnode_init(1, buddy_root_order, (void *)buddy_start);
}