#include <pmm.h>

lock_t buddy_lk = LOCK_INIT();

void buddy_init()
{
    Log("heap.start:%p", heap.start);
    Log("heap.end:%p", heap.end);
    pm_start = (uintptr_t)heap.start;
    pm_end = (uintptr_t)heap.end;
    pmm_size = pm_end - pm_start;
    Log("max pages:%d", pmm_size / PAGE_SIZE);

    pm_start = align(pm_start, PAGE_SIZE);

    total_page_num = pmm_size / PAGE_SIZE;
    max_buddy_node_num = 2 * total_page_num;
    buddy = (buddy_node *)((uint8_t *)pm_end - (max_buddy_node_num + 2) * sizeof(buddy_node));
    buddy[1].size = pmm_size / PAGE_SIZE;
    buddy[1].status = BUD_AVAILABLE;
    size_t cur = 1;
    while (2 * cur + 1 <= max_buddy_node_num)
    {
        buddy[2 * cur].size = buddy[2 * cur + 1].size = buddy[cur].size / 2;
        cur++;
    }

    Log("max_buddy_node_num:%d", max_buddy_node_num);
    Log("total_page_num:%d", total_page_num);
    Log("pm_start:%p", pm_start);
    Log("pm_end:%p", pm_end);
}

size_t get_one_buddy_node(size_t cur, size_t size)
{
    //Log("buddy[%d].status = %d , buddy[%d].size = %d ", cur, buddy[cur].status, cur, buddy[cur].size);
    if (buddy[cur].status == BUD_AVAILABLE && buddy[cur].size >= size)
    {
        if (buddy[cur].size < 2 * size)
        {
            buddy[cur].status = BUD_USED;
            size_t parent = cur / 2;
            while (buddy[2 * parent].status == BUD_USED && buddy[2 * parent + 1].status == BUD_USED)
            {
                buddy[parent].status = BUD_USED;
                parent /= 2;
            }
            return cur;
        }
        else
        {
            if (buddy[2 * cur].status == BUD_AVAILABLE)
                return get_one_buddy_node(2 * cur, size);
            if (buddy[2 * cur + 1].status == BUD_AVAILABLE)
                return get_one_buddy_node(2 * cur + 1, size);

            //buddy[cur].status = BUD_SPLITTED;
            buddy[2 * cur].status = buddy[2 * cur + 1].status = BUD_AVAILABLE;
            return get_one_buddy_node(2 * cur, size);
        }
    }
    Log("Get buddy node failed! cur = %d size = %d", cur, size);
    assert(0);
    return 0;
}

void *buddy_alloc(size_t size)
{
    void *ret = NULL;
    size_t obj_buddy_node = 0;
    size = size / PAGE_SIZE;
    Log("buddy_alloc %d page ", size);

    lock(&buddy_lk);
    obj_buddy_node = get_one_buddy_node(1, size);
    Log("Got buddy node id :%d", obj_buddy_node);

    ret = (void *)PAGE(size * (obj_buddy_node % (1 << log(obj_buddy_node))));
    Log("ret:%p", ret);

    unlock(&buddy_lk);

    assert((uintptr_t)ret % size == 0);
    return ret;
}