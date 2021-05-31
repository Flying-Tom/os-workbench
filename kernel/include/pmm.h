#include <common.h>

/* const macro */
#define MAX_CPU_NUM 8
#define CPU_CUR (uint8_t)(cpu_current())
#define CPU_NUM (uint8_t)(cpu_count())

/* slab system */
#define PAGE_SIZE (4 KB)
#define PAGE_LMASK ~(uintptr_t)(PAGE_SIZE - 1)
#define PAGE_RMASK (uintptr_t)(PAGE_SIZE - 1)
#define PAGE_ORDER 12
#define MAX_SLAB_TYPE 6

typedef struct page_header
{
    void *prev;
    void *next;
    void *entry;
    uint8_t cpu;
    uint8_t type;
    short units_remaining;
} page_header;

void *slab_start, *slab_end;
void slab_init(uint8_t cpu, void *start, size_t size);
void *slab_alloc(size_t size);
void slab_free(void *ptr);

/* buddy system */
#define MAX_BUDDY_NODE_TYPE 20
#define MAX_BUD_ORDER 24
#define MAX_BUD_SIZE (1 << MAX_BUD_ORDER)
#define MAX_BUD_LMASK ~(uintptr_t)(MAX_BUD_SIZE - 1)

typedef struct buddy_node
{
    void *addr;
    enum
    {
        BUD_EMPTY,
        BUD_SPLITTED,
        BUD_FULL
    };
    uint8_t status;
    uint8_t order;
} buddy_node;

void buddy_init(void *start, void *end);
void *buddy_alloc(size_t size);
void buddy_free(void *ptr);

/* Tool func and macro */

uint8_t log(size_t x);
size_t binalign(size_t size);

#define BREAKPOINT(a) Log("BREAKPOINT:" #a "\n")
#define align(base, offset) (((base + offset - 1) / offset) * offset) // Right align