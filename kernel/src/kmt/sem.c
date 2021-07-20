#include <kmt.h>

void semmod_init()
{
    sem_t fuck;
    printf("%d\n", sizeof(fuck->tasks) / sizeof(task_t*));
}

void sem_init(sem_t* sem, const char* name, int value)
{
    kmt->spin_init(&sem->lock, name);

    for (int i = 0; i < MAX_SEM_TASK_NUM; i++) {
        sem->tasks[i] = NULL;
    }
    sem->value = value;
    sem->tail = sem->head = 0;
}
void sem_wait(sem_t* sem)
{
    kmt->spin_lock(&sem->lock);
    bool flag = false;
    sem->value--;
    if (sem->value < 0) {
        sem->tasks[sem->tail] = cur_task;
        sem->tail = (sem->tail + 1) % MAX_SEM_TASK_NUM;
        cur_task->status = TASK_WAITTING;
        flag = true;
    }
    kmt->spin_unlock(&sem->lock);
    if (flag) {
        yield();
        while (cur_task->status != TASK_AVAILABLE)
            ;
    }

    //panic(cur_task->status == TASK_DEAD);
}
void sem_signal(sem_t* sem)
{
    kmt->spin_lock(&sem->lock);
    sem->value++;
    if (sem->tasks[sem->head] != NULL) {
        sem->tasks[sem->head] = NULL;
        sem->head = (sem->head + 1) % MAX_SEM_TASK_NUM;
    }
    kmt->spin_unlock(&sem->lock);
}
