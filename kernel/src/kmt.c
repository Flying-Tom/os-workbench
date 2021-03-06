#include <common.h>
#include <kmt.h>

spinlock_t task_lock;
spinlock_t os_trap_lock;

static Context* kmt_context_save(Event e, Context* c)
{
    kmt->spin_lock(&os_trap_lock);

    if (pre_task != NULL) {

        panic_on(pre_task->pause == 0, "pre_task should be paused");
        pre_task->pause = 0;
        pre_task = NULL;
    }
    cur_task->context = c;
    kmt->spin_unlock(&os_trap_lock);
    return NULL;
}

static Context* kmt_schedule(Event e, Context* c)
{
    kmt->spin_lock(&os_trap_lock);
    int cnt = -1, id = 0;
    if (task_cnt > 0) {
        if (cur_task == &idle_task) {
            id = 0;
            cnt = task_cnt;
        } else {
            id = cur_task->id;
            cnt = task_cnt - 1;
        }

        do {
            if (--cnt < 0)
                break;
            id = (id + 1) % task_cnt;

        } while (tasks[id]->status != TASK_RUNNING || tasks[id]->running == 1 || atomic_xchg(&tasks[id]->pause, 1));
    }

    cur_task->running = 0;
    assert(pre_task == NULL);

    if (cur_task != &idle_task) {
        panic_on(cur_task->pause == 0, "cur_task->pause should be 1");
        pre_task = cur_task;
    }

    if (cnt >= 0) {
        panic_on(tasks[id]->status != TASK_RUNNING, "tasks[id] should be running");
        cur_task = tasks[id];

    } else
        cur_task = &idle_task;

    cur_task->running = 1;
    kmt->spin_unlock(&os_trap_lock);
    return cur_task->context;
}

static void kmt_init()
{
    Log("kmt_init start");

    spinmod_init();
    semmod_init();
    task_cnt = 0;
    kmt->spin_init(&task_lock, "task_lock");
    kmt->spin_init(&os_trap_lock, "os_trap_lock");

    for (int i = 0; i < MAX_CPU_NUM; i++) {
        cur_tasks[i] = &idle_tasks[i];
        pre_tasks[i] = NULL;
        idle_tasks[i] = (task_t) {
            .status = TASK_RUNNING,
            .running = 0,
            .pause = 0,
            .id = -1,
            .stack = pmm->alloc(STACK_SIZE),
            .context = kcontext((Area) { .start = (void*)(&idle_tasks[i].stack), .end = (void*)((char*)(&idle_tasks[i].stack) + STACK_SIZE) }, NULL, NULL),
        };
    }

    os->on_irq(INT_MIN, EVENT_NULL, kmt_context_save); // ??????????????????
    os->on_irq(INT_MAX, EVENT_NULL, kmt_schedule); // ??????????????????
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
