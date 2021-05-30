#include <pmm.h>

static int buddy_lk = 0;
static uint8_t buddy_root_order;
static void *buddy_area_start;
static buddy_node *buddy;

static inline void *buddy_alloc_search(int id, uint8_t cur_order, uint8_t tar_order)
{
    if (cur_order < PAGE_ORDER || buddy[id].status == BUD_FULL || tar_order > buddy[id].order)
        return NULL;

    void *ret = buddy_alloc_search(id * 2, cur_order - 1, tar_order);
    
    if (ret == NULL)
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

static inline void buddy_free_search(int id, uint8_t cur_order, void *tar_ptr)
{
    assert(cur_order >= PAGE_ORDER);
    if (buddy[id].addr == tar_ptr && buddy[id].status == BUD_FULL)
    {
        buddy[id].order = cur_order;
        buddy[id].status = BUD_EMPTY;
        return;
    }
    if (tar_ptr < buddy[id * 2 + 1].addr)
        buddy_free_search(id * 2, cur_order - 1, tar_ptr);
    else
        buddy_free_search(id * 2 + 1, cur_order - 1, tar_ptr);

    if (buddy[id * 2].status == BUD_EMPTY && buddy[id * 2 + 1].status == BUD_EMPTY)
        buddy[id].status = BUD_EMPTY;

    buddy[id].order = max(buddy[id * 2].order, buddy[id * 2 + 1].order);
}

void *buddy_alloc(size_t size)
{
    Log("buddy alloc %d bytes", size);
    void *ret = NULL;
    lock(&buddy_lk);
    ret = buddy_alloc_search(1, buddy_root_order, (uint8_t)(log(size - 1) + 1));
    unlock(&buddy_lk);
    return ret;
}

void buddy_free(void *ptr)
{
    //lock(&buddy_lk);
    buddy_free_search(1, buddy_root_order, ptr);
    //unlock(&buddy_lk);
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
    size_t budnode_area_size = ((uintptr_t)(end - start) >> (MAX_BUD_ORDER - PAGE_ORDER - 1)) * sizeof(buddy_node);
    buddy = (buddy_node *)start;
    buddy_area_start = (void *)(((uintptr_t)buddy + budnode_area_size + MAX_BUD_SIZE - 1) & MAX_BUD_LMASK);
    Log("buddy system really used space start from : %p ", buddy_area_start);

    size_t buddy_max_size = MAX_BUD_SIZE;
    while (buddy_area_start + buddy_max_size <= end)
        buddy_max_size <<= 1;
    buddy_max_size >>= 1;
    buddy_root_order = (uint8_t)log(buddy_max_size);
    buddy--;
    Log("buddy_root_order:%d", buddy_root_order);
    budnode_init(1, buddy_root_order, buddy_area_start);
}