#include <common.h>

#define DEBUG_LOCAL

static trap_handler_t trap_handlers[SEQ_MAX][TRAP_HANDLER_MAX_NUM];

static void trap_init()
{
    for (int i = SEQ_MIN; i < SEQ_MAX; i++) {
        for (int j = 0; j < TRAP_HANDLER_MAX_NUM; j++) {
            trap_handlers[i][j].handler = NULL;
            trap_handlers[i][j].status = 0;
        }
    }
}

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
    trap_init();
    pmm->init();
    kmt->init();

    Log("Start Testcase");
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
    trap_init();
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

    for (int i = SEQ_MIN; i < SEQ_MAX; i++) {
        for (int j = 0; j < TRAP_HANDLER_MAX_NUM; j++) {
            if (trap_handlers[i][j].status == 1) {
                if (trap_handlers[i][j].event == EVENT_NULL || trap_handlers[i][j].event == ev.event) {
                    Context* r = trap_handlers[i][j].handler(ev, context);
                    panic_on(r && ret, "returning multiple contexts");
                    if (r)
                        ret = r;
                }
            } else
                break;
        }
    }
    panic_on(!ret, "returning NULL context");
    //panic_on(sane_context(ret), "returning to invalid context");
    return ret;
}


static void os_on_irq(int seq, int event, handler_t handler)
{
    int cnt;
    for (cnt = SEQ_MIN; cnt < TRAP_HANDLER_MAX_NUM; cnt++) {
        if (trap_handlers[seq][cnt].status == 0) {
            break;
        }
    }

    assert(cnt < TRAP_HANDLER_MAX_NUM);
    trap_handlers[seq][cnt].status = 1;
    trap_handlers[seq][cnt].seq = seq;
    trap_handlers[seq][cnt].event = event;
    trap_handlers[seq][cnt].handler = handler;
}

MODULE_DEF(os) = {
    .init = os_init,
    .run = os_run,
    .trap = os_trap,
    .on_irq = os_on_irq,
};
