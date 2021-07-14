#include <kmt.h>

#define MAX_TASK_NUM 64

task_t* cur_task[MAX_CPU_NUM];
task_t* tasks[MAX_CPU_NUM][MAX_TASK_NUM];

int task_num[MAX_CPU_NUM];

static void kmt_init()
{
    Log("kmt_init start");
    for (int i = 0; i < MAX_CPU_NUM; i++) {
        cur_task[i] = NULL;
        for (int j = 0; j < MAX_TASK_NUM; j++) {
            tasks[i][j] = NULL;
        }
        task_num[i] = 0;
    }

    /*
    os->on_irq(INT_MIN, EVENT_NULL, kmt_context_save); // 总是最先调用
    os->on_irq(INT_MAX, EVENT_NULL, kmt_schedule); // 总是最后调用
    */
}

static int create(task_t* task, const char* name, void (*entry)(void* arg), void* arg)
{
    task->name = name;
    Area stack = (Area) { .start = &task->stack, .end = (void*)((char*)(&task->stack) + STACK_SIZE) };
    Log("stack start:%p\n", stack.start);
    Log("stack end:%p\n", stack.end);
    task->context = kcontext(stack, entry, arg);

    int temp = INT32_MAX, cpu_pos = -1, task_pos = 0;

    for (int i = 0; i < CPU_NUM; i++) {
        if (task_num[i] <= temp) {
            temp = task_num[i];
            cpu_pos = i;
        }
    }

    task_num[cpu_pos]++;

    for (task_pos = 0; task_pos < MAX_TASK_NUM; task_pos++) {
        if (tasks[cpu_pos][task_pos] == NULL) {
            tasks[cpu_pos][task_pos] = task;
            task->cpu = cpu_pos;
            break;
        }
    }
    return 0;
}

static void teardown(task_t* task)
{
}

void spin_init(spinlock_t* lk, const char* name)
{
}

void spin_lock(spinlock_t* lk)
{
}

void spin_unlock(spinlock_t* lk)
{
}

void sem_init(sem_t* sem, const char* name, int value)
{
}

void sem_wait(sem_t* sem)
{
}

void sem_signal(sem_t* sem)
{
}

MODULE_DEF(kmt) = {
    .init = kmt_init,
    .create = create,
    .teardown = teardown,
    .spin_init = spin_init,
    .spin_lock = spin_lock,
    .spin_unlock = spin_unlock,
    .sem_init = sem_init,
    .sem_wait = sem_wait,
    .sem_signal = sem_signal,
};
