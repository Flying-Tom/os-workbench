#include <common.h>

#define BREAKPOINT(a) Log("BREAKPOINT:" #a "\n")
#define align(base, offset) (((base + offset - 1) / offset) * offset) // Right align

/* const macro */
#define PAGE_SIZE (4 KB)
#define PAGE_LMASK ~(uintptr_t)(PAGE_SIZE - 1)
#define PAGE_RMASK (uintptr_t)(PAGE_SIZE - 1)
#define PAGE_ORDER 12
#define MAX_CPU_NUM 8
#define MAX_SLAB_TYPE 6
#define CPU_CUR (uint8_t)(cpu_current())

/* page header */
#define PAGE_HEADER(a) (page_header *)(pm_start + (a + 1) * PAGE_SIZE - sizeof(page_header))
#define PAGE(a) (pm_start + a * PAGE_SIZE)
#define slab_max_items(size) ((PAGE_SIZE - sizeof(page_header)) / size)

/* buddy system*/

#define MAX_BUDDY_NODE_TYPE 20
#define MAX_BUD_ORDER 24
#define MAX_BUD_SIZE (1 << MAX_BUD_ORDER)
#define MAX_BUD_LMASK ~(uintptr_t)(MAX_BUD_SIZE - 1)

/*------------------------------------------*/

uint8_t cpu_id, cpu_num;

typedef struct page_header
{
    void *prev;
    void *next;
    void *entry;
    uint8_t cpu;
    uint8_t type;
    short units_remaining;
} page_header;

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

/* slab system */
void *slab_start, *slab_end;
void slab_init(uint8_t cpu, void *start, size_t size);
void *slab_alloc(size_t size);
void slab_free(void *ptr);

/* buddy system */
void buddy_init(void *start, void *end);
void *buddy_alloc(size_t size);
void buddy_free(void *ptr);

/* Tool func */

size_t log(size_t x);
size_t binalign(size_t size);
