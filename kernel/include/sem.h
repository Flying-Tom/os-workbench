#ifndef SEM_H
#define SEM_H

#include <kmt.h>
#include <spinlock.h>

void sem_init(sem_t* sem, const char* name, int value);
void sem_wait(sem_t* sem);
void sem_signal(sem_t* sem);

#endif