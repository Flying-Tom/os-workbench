#include <common.h>

#define DEBUG_LOCAL

#ifdef DEBUG_LOCAL
sem_t empty, fill;
#define P kmt->sem_wait
#define V kmt->sem_signal

void producer(void* arg)
{
    while (1) {
        P(&empty);
        putch('(');
        V(&fill);
    }
}
void consumer(void* arg)
{
    while (1) {
        P(&fill);
        putch(')');
        V(&empty);
    }
}

static void os_init()
{
    pmm->init();
    kmt->init();
    kmt->sem_init(&empty, "empty", 5); // 缓冲区大小为 5
    kmt->sem_init(&fill, "fill", 0);
    for (int i = 0; i < 4; i++) // 4 个生产者
        kmt->create(pmm->alloc(sizeof(task_t)), "producer", producer, NULL);
    for (int i = 0; i < 5; i++) // 5 个消费者
        kmt->create(pmm->alloc(sizeof(task_t)), "consumer", consumer, NULL);
}
#else
static void os_init()
{
    pmm->init();
    kmt->init();
}
#endif

static void os_run()
{
    /*
    for (const char *s = "Hello World from CPU #*\n"; *s; s++)
    {
        putch(*s == '*' ? '0' + cpu_current() : *s);
    }
    */
    printf("Tests Start!\n");

    //iset(true);
    //iset(false);

    printf("Tests done!\n");

    //assert(0);
    while (1)
        ;
}

static Context* os_trap(Event ev, Context* context)
{
    Context* ret = NULL;

    /*
    for (int i = SEQ_MIN; i < SEQ_MAX; i++) {
        if (h.event == EVENT_NULL || h.event == ev.event) {
            Context* r = h.handler(ev, ctx);
            panic_on(r && ret, "returning multiple contexts");
            if (r)
                ret = r;
        }
    }
    panic_on(!ret, "returning NULL context");
    panic_on(sane_context(ret), "returning to invalid context");
    */
    assert(0);
    return ret;
}

/*
static Context* os_trap(Event ev, Context* ctx)
{
    Context* next = NULL;
    for (auto& h : handlers_sorted_by_seq) {
        if (h.event == EVENT_NULL || h.event == ev.event) {
            Context* r = h.handler(ev, ctx);
            panic_on(r && next, "returning multiple contexts");
            if (r)
                next = r;
        }
    }
    panic_on(!next, "returning NULL context");
    panic_on(sane_context(next), "returning to invalid context");
    return next;
}
*/

static void os_on_irq(int seq, int event, handler_t handler)
{
}

MODULE_DEF(os) = {
    .init = os_init,
    .run = os_run,
    .trap = os_trap,
    .on_irq = os_on_irq,
};
