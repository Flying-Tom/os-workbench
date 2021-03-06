#ifndef KMT_H
#define KMT_H

#include <common.h>

task_t idle_tasks[MAX_CPU_NUM];
task_t* cur_tasks[MAX_CPU_NUM];
task_t* pre_tasks[MAX_CPU_NUM];
task_t* tasks[1024];

int task_cnt;

#define idle_task idle_tasks[CPU_CUR]
#define cur_task cur_tasks[CPU_CUR]
#define pre_task pre_tasks[CPU_CUR]

/* spinlock */
void spinmod_init();
void spinlock_init(spinlock_t* lk, const char* name);
void spinlock_acquire(spinlock_t* lk);
void spinlock_release(spinlock_t* lk);
uint8_t spinlock_holding(spinlock_t* lk);
void spinlock_pushcli();
void spinlock_popcli();

/* semaphore */

void semmod_init();
void sem_init(sem_t* sem, const char* name, int value);
void sem_wait(sem_t* sem);
void sem_signal(sem_t* sem);

#ifdef KMT_DEBUG
#define Log(...) _Log(__VA_ARGS__)
#else
#define Log(...)
#endif

#endif