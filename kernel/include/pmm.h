#ifndef PMM_H
#define PMM_H
#include <common.h>

/* slab system */
#define PAGE_SIZE (4 KB)
#define PAGE_LMASK ~(uintptr_t)(PAGE_SIZE - 1)
#define PAGE_RMASK (uintptr_t)(PAGE_SIZE - 1)
#define PAGE_ORDER 12
#define MAX_SLAB_TYPE 7

typedef struct page_header {
    struct page_header* prev;
    struct page_header* next;
    void* entry;
    uint8_t cpu;
    uint8_t type;
    short units_remaining;
} page_header;

void *slab_start, *slab_end;
void slab_init(uint8_t cpu, void* start, size_t size);
void* slab_alloc(uint8_t order);
void slab_free(void* ptr);

/* buddy system */
#define MAX_BUDDY_NODE_TYPE 20
#define MAX_BUD_ORDER 24
#define MAX_BUD_SIZE (1 << MAX_BUD_ORDER)
#define MAX_BUD_LMASK ~(uintptr_t)(MAX_BUD_SIZE - 1)

enum {
    BUD_EMPTY,
    BUD_SPLITTED,
    BUD_FULL
};

typedef struct buddy_node {
    void* addr;
    uint8_t status;
    uint8_t order;
} buddy_node;

void buddy_init(void* start, void* end);
void* buddy_alloc(uint8_t order);
void buddy_free(void* ptr);

/* Tool func and macro */

uint8_t log(size_t x);
uint8_t calorder(size_t size);

#define BREAKPOINT(a) Log("BREAKPOINT:" #a "\n")
#define align(base, offset) (((base + offset - 1) / offset) * offset) // Right align

inline void lock(lock_t* lk)
{
    while (atomic_xchg(lk, 1))
        ;
}
inline void unlock(lock_t* lk)
{
    atomic_xchg(lk, 0);
}

#endif