#include <kmt.h>

void sem_init(sem_t* sem, const char* name, int value)
{
    sem->value = value;
    kmt->spin_init(&sem->lock, name);

    for (int i = 0; i < MAX_TASK_NUM; i++) {
        sem->tasks[i] = NULL;
    }
    sem->tail = sem->head = 0;
}
void sem_wait(sem_t* sem)
{
    kmt->spin_lock(&sem->lock);
    if (sem->value <= 0) {
        sem->tasks[sem->tail] = cur_task[CPU_CUR];
        sem->tail = (sem->tail + 1) % MAX_TASK_NUM;
    }

    while (sem->value <= 0) {
        kmt->spin_unlock(&sem->lock);
        yield();
        kmt->spin_lock(&sem->lock);
    }
    sem->value--;
    kmt->spin_unlock(&sem->lock);
}
void sem_signal(sem_t* sem)
{
    kmt->spin_lock(&sem->lock);
    sem->value++;
    if (sem->tasks[sem->head] != NULL) {
        sem->tasks[sem->head] = NULL;
        sem->head = (sem->head + 1) % MAX_TASK_NUM;
    }
    kmt->spin_unlock(&sem->lock);
}
