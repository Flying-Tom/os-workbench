#include <pmm.h>

lock_t pm_global_lk = LOCK_INIT();

void *global_alloc(size_t size)
{
    /*
    lock(&pm_global_lk);
    void *ret = NULL;
    uint8_t order = 0;
    order = log(size / PAGE_SIZE) + 1;
    Log("global_alloc %d Bytes  Its order:%d", size, order);
    ret = (void *)PAGE(get_one_block(order));
    unlock(&pm_global_lk);
    return ret;
    */
    lock(&pm_global_lk);
    void *ret = NULL;
    pm_cur -= size;
    ret = (void *)pm_cur;
    unlock(&pm_global_lk);
    return ret;
}

static void *kalloc(size_t size)
{
    void *ret = NULL;
    Log("kalloc: %d", size);
    assert(size > 0);
    if (size >= PAGE_SIZE)
    {
        lock(&pm_global_lk);
        size = 1 << (log(size - 1) + 1);
        ret = global_alloc(size);
        Log("global_alloc size:%d ret:%p", size, ret);
        assert((uintptr_t)ret % size == 0);
        unlock(&pm_global_lk);
    }
    else
        ret = slab_alloc(size);
    return ret;
}

static void kfree(void *ptr)
{
    uintptr_t page_addr = (uintptr_t)ptr / PAGE_SIZE * PAGE_SIZE;
    page_header *cur = (page_header *)(page_addr + PAGE_SIZE - sizeof(page_header));

    uint64_t items = ((uintptr_t)ptr - page_addr) / (1 << cur->slab_type);
    //lock(&page_lk);
    cur->bitmap[items / 64] ^= (1 << (items % 64));
    //unlock(&page_lk);
}

static void pmm_init()
{
    cpu_num = cpu_count();
    assert(cpu_num <= MAX_CPU_NUM);

    pm_start = (uintptr_t)heap.start;
    pm_end = (uintptr_t)heap.end;
    Log("pm_start:%p aligned pm_start:%p", pm_start, align(pm_start, PAGE_SIZE));
    Log("pm_end:%p", pm_end);
    pm_start = align(pm_start, PAGE_SIZE);
    pm_cur = pm_end;

    total_page_num = (pm_end - pm_start) / PAGE_SIZE;
    Log("total_page_num:%d", total_page_num);
    max_order = log(total_page_num);
    Log("max_order:%d", max_order);

    assert((pm_end - pm_start) % PAGE_SIZE == 0);
    Log("Total pages:%d", total_page_num);
    Log("pmm_init finished");
}

MODULE_DEF(pmm) = {
    .init = pmm_init,
    .alloc = kalloc,
    .free = kfree,
};
