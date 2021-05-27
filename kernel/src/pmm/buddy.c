#include <pmm.h>

#define MAX_BUD_ORDER 24
#define MAX_BUD_SIZE (1 << MAX_BUD_ORDER)

lock_t buddy_lk = 0;
uint8_t buddy_root_order;
void *buddy_area_start;

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
void buddy_init(void *start, void *end)
{
    Log("buddy system starts from %p to %p", start, end);
    size_t budnode_area_size = ((uintptr_t)(end - start) >> (MAX_BUD_ORDER - PAGE_ORDER)) * sizeof(buddy_node);
    buddy = (buddy_node *)start;
    buddy_area_start = (void *)(align(((uintptr_t)buddy + budnode_area_size), MAX_BUD_SIZE));
    Log("buddy system really used space start from : %p ", buddy_area_start);

    size_t buddy_max_size = MAX_BUD_ORDER;
    while (buddy_area_start + buddy_max_size <= end)
        buddy_max_size <<= 1;
    buddy_max_size >>= 1;
    buddy_root_order = log(buddy_max_size);
    Log("buddy_root_order:%d", buddy_root_order);
    budnode_init(1, buddy_root_order, (void *)buddy_area_start);
}