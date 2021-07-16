#ifndef SEM_H
#define SEM_H

#include <kmt.h>
#include <spinlock.h>

struct semaphore {
    int value;
    spinlock_t lock;
    task_t* tasks[MAX_TASK_NUM];
    int head;
    int tail;
};


void sem_init(sem_t* sem, const char* name, int value);
void sem_wait(sem_t* sem);
void sem_signal(sem_t* sem);

#endif