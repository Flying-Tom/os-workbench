#include <common.h>
#include <kmt.h>

spinlock_t task_lock;

static Context* kmt_context_save(Event e, Context* c)
{
    kmt->spin_lock(&task_lock);

    if (cur_task != NULL) {

        panic_on(cur_task->pause == 0, "cur_task should be paused");
        cur_task->pause = 0;
        cur_task = NULL;
    }
    nxt_task->context = c;
    kmt->spin_unlock(&task_lock);
    return NULL;
}

static Context* kmt_schedule(Event e, Context* c)
{
    kmt->spin_lock(&task_lock);
    int cnt = -1, id = 0;
    if (task_cnt > 0) {
        if (nxt_task == idle_task) {
            id = 0;
            cnt = task_cnt;
        } else {
            id = nxt_task->id;
            cnt = task_cnt - 1;
        }

        do {
            if (--cnt < 0)
                break;
            id = (id + 1) % task_cnt;

        } while (tasks[id]->status != TASK_RUNNING || tasks[id]->running == 1 || atomic_xchg(&tasks[id]->pause, 1));
    }

    nxt_task->running = 0;

    if (nxt_task != idle_task) {
        panic_on(nxt_task->pause == 0, "nxt_task should be paused");
        cur_task = nxt_task;
    }

    if (cnt >= 0) {
        panic_on(tasks[id]->status != TASK_RUNNING, "tasks[id] should be running");
        tasks[id]->running = 1;
        nxt_task = tasks[id];

    } else {
        idle_task->running = 1;
        nxt_task = idle_task;
    }
    kmt->spin_unlock(&task_lock);
    return nxt_task->context;
}

static void kmt_init()
{
    Log("kmt_init start");

    spinmod_init();
    semmod_init();
    task_cnt = 0;
    kmt->spin_init(&task_lock, "task_lock");

    for (int i = 0; i < MAX_CPU_NUM; i++) {
        idle_task[i] = (task_t*)pmm->alloc(sizeof(task_t));
        nxt_tasks[i] = idle_tasks[i];
        cur_tasks[i] = NULL;
        *idle_tasks[i] = (task_t) {
            .status = TASK_RUNNING,
            .running = 0,
            .pause = 0,
            .id = -1,
            .stack = pmm->alloc(STACK_SIZE),
            .context = kcontext((Area) { .start = (void*)(idle_tasks[i]->stack), .end = (void*)((char*)(idle_tasks[i]->stack) + STACK_SIZE) }, NULL, NULL),
        };
    }

    os->on_irq(INT_MIN, EVENT_NULL, kmt_context_save); // 总是最先调用
    os->on_irq(INT_MAX, EVENT_NULL, kmt_schedule); // 总是最后调用
    Log("kmt_init finished");
}

static int create(task_t* task, const char* name, void (*entry)(void* arg), void* arg)
{
    task->name = name;
    task->stack = pmm->alloc(STACK_SIZE);
    Area stack = (Area) { .start = (void*)(task->stack), .end = (void*)((char*)(task->stack) + STACK_SIZE) };
    task->context = kcontext(stack, entry, arg);

    task->running = 0;
    task->pause = 0;

    kmt->spin_lock(&task_lock);

    task->id = task_cnt;
    tasks[task_cnt++] = task;
    task->status = TASK_RUNNING;

    kmt->spin_unlock(&task_lock);
    return 0;
}

static void teardown(task_t* task)
{
    kmt->spin_lock(&task_lock);
    task->status = TASK_DEAD;
    panic_on(task->running == 1, "task shouldn't be running");
    pmm->free(task->stack);
    kmt->spin_unlock(&task_lock);
}

MODULE_DEF(kmt) = {
    .init = kmt_init,
    .create = create,
    .teardown = teardown,
    .spin_init = spinlock_init,
    .spin_lock = spinlock_acquire,
    .spin_unlock = spinlock_release,
    .sem_init = sem_init,
    .sem_wait = sem_wait,
    .sem_signal = sem_signal,
};
