#ifndef SPINLOCK_H
#define SPINLOCK_H

struct spinlock {
    const char* name;
    uint8_t locked;
    uint8_t cpu;
};

typedef struct spinlock spinlock_t;

void spinlock_init(spinlock_t* lk, const char* name);
void spinlock_acquire(spinlock_t* lk);
void spinlock_release(spinlock_t* lk);
uint8_t spinlock_holding(spinlock_t* lk);
void spinlock_pushcli();
void spinlock_popcli();

#endif