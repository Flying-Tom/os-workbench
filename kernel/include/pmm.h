#include <common.h>

#define BREAKPOINT(a) Log("BREAKPOINT:" #a "\n")
#define align(base, offset) (((base + offset - 1) / offset) * offset) // Right align

/* const macro */
#define PAGE_SIZE (4 KB)
#define PAGE_ORDER 12
#define MAX_CPU_NUM 8
#define MAX_SLAB_TYPE 9
#define CPU_CUR cpu_current()

/* page header */
#define PAGE_HEADER(a) (page_header *)(pm_start + (a + 1) * PAGE_SIZE - sizeof(page_header))
#define PAGE(a) (pm_start + a * PAGE_SIZE)
#define slab_max_items(size) ((PAGE_SIZE - sizeof(page_header)) / size)

/* buddy system*/
#define MAX_BUDDY_NODE_TYPE 20

/* bitmap */
#define BITMAP_FULL(page, id) (page->bitmap[id] + 1ULL == 0ULL)

/*------------------------------------------*/

size_t pm_size;
uint8_t cpu_id, cpu_num;

typedef struct page_header
{
    void *prev;
    void *next;
    void *entry;
    uint8_t cpu;
    uint8_t type;
    size_t units_remaining;
} page_header;

typedef struct Cache
{
    void *entry;
    int lk;
} Cache;

enum
{
    BUD_EMPTY,
    BUD_SPLITTED,
    BUD_FULL
};

typedef struct buddy_node
{
    void *addr;
    uint8_t status;
    uint8_t order;
} buddy_node;

buddy_node *buddy;

/* slab system */
void *slab_start, slab_end;
void slab_init(uintptr_t start, uintptr_t end);
void *slab_alloc(size_t size);

/* buddy system */
void buddy_init(uintptr_t start, uintptr_t end);
void *buddy_alloc(size_t size);

/* Tool func */

size_t log(size_t x);
size_t binalign(size_t size);
