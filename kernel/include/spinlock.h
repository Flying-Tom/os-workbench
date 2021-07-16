#ifndef SPINLOCK_H
#define SPINLOCK_H

void spinlock_init(spinlock_t* lk, const char* name);
void spinlock_acquire(spinlock_t* lk);
void spinlock_release(spinlock_t* lk);
uint8_t spinlock_holding(spinlock_t* lk);
void spinlock_pushcli();
void spinlock_popcli();

#endif