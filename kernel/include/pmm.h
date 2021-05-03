#include <common.h>
#include <lock.h>

#define BREAKPOINT(a) Log("BREAKPOINT:" #a "\n")
#define align(base, offset) (((base + offset - 1) / offset) * offset) // Right align

/* const macro */
#define PAGE_SIZE (4 KB)
#define MAX_CPU_NUM 8
#define MAX_SLAB_TYPE 12

/* page header */
#define PAGE_HEADER(a) (page_header *)(pm_start + (a + 1) * PAGE_SIZE - sizeof(page_header))
#define PAGE(a) (pm_start + a * PAGE_SIZE)
#define slab_max_items(size) ((PAGE_SIZE - sizeof(page_header)) / size)

/* buddy system*/
#define MAX_BUDDY_NODE_TYPE 20

/* bitmap */
#define BITMAP_FULL(page, id) (page->bitmap[id] + 1ULL == 0ULL)

/*------------------------------------------*/
lock_t pm_global_lk = LOCK_INIT();

uintptr_t pm_start, pm_end;
size_t pmm_size;
uint8_t cpu_id, cpu_num;
size_t total_page_num;
size_t max_buddy_node_num;

typedef struct page_header
{
    size_t id;
    uint8_t parent_cpu_id;
    uint64_t bitmap[64];
    uint8_t slab_type;
    struct page_header *next;
} page_header;

typedef struct Cache
{
    int size;
    int num_of_slab;
    page_header *slab_free;
} Cache;
Cache cache[MAX_CPU_NUM][8];

enum
{
    BUD_EMPTY,
    BUD_AVAILABLE,
    BUD_USED,
    BUD_SPLITTED
};

typedef struct buddy_node
{
    uint8_t status;
    size_t size;
} buddy_node;

buddy_node *buddy;

/* buddy system */
void buddy_init();
size_t get_one_buddy_node(size_t cur, size_t size);
void *buddy_alloc(size_t size);
void *slab_alloc(size_t size);

/* Tool func */

size_t log(size_t x);
size_t binalign(size_t size);

size_t log(size_t x)
{
    size_t ret = 0;
    while (x > 1)
    {
        x >>= 1;
        ret++;
    }
    return ret;
}

size_t binalign(size_t size)
{
    bool flag = true;
    size_t ret = 1;
    assert(size > 0);
    while (size != 1)
    {
        if (size & 1)
            flag = false;
        ret <<= 1;
        size >>= 1;
    }
    ret = flag == true ? ret : ret << 1;
    return ret;
}