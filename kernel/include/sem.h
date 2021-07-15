#ifndef SEMAPHORE_H
#define SEMAPHORE_H

struct semaphore {
};

void sem_init(sem_t* sem, const char* name, int value);
void sem_wait(sem_t* sem);
void sem_signal(sem_t* sem);

#endif