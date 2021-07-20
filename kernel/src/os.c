#include <common.h>
#include <devices.h>

//#define DEBUG_LOCAL

void handler_NULL()
{
    printf("Enter undefined handler!\n");
    assert(0);
}

trap_t trap_head = (trap_t) {
    .seq = INT_MIN,
    .event = 0,
    .handler = (handler_t)handler_NULL,
    .next = NULL,
};

static void trap_init()
{
}

#ifdef DEBUG_LOCAL
static void __attribute__((used)) tty_reader(void* arg)
{
    device_t* tty = dev->lookup(arg);
    char cmd[128], resp[128], ps[16];
    snprintf(ps, 16, "(%s) $ ", arg);
    while (1) {
        tty->ops->write(tty, 0, ps, strlen(ps));
        int nread = tty->ops->read(tty, 0, cmd, sizeof(cmd) - 1);
        cmd[nread] = '\0';
        sprintf(resp, "tty reader task: got %d character(s).\n", strlen(cmd));
        tty->ops->write(tty, 0, resp, strlen(resp));
    }
}

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
    dev->init();
    printf("%d\n", task_cnt);
    assert(0);
    
    printf("Start Testcase\n");
    kmt->sem_init(&empty, "empty", 5); // 缓冲区大小为 5
    kmt->sem_init(&fill, "fill", 0);

    /*
    for (int i = 0; i < 5; i++) // 5 个消费者
        kmt->create(pmm->alloc(sizeof(task_t)), "consumer", consumer, NULL);
    for (int i = 0; i < 4; i++) // 4 个生产者
        kmt->create(pmm->alloc(sizeof(task_t)), "producer", producer, NULL);
        */

    //kmt->create(pmm->alloc(sizeof(task_t)), "tty_reader", tty_reader, "tty1");
    //kmt->create(pmm->alloc(sizeof(task_t)), "tty_reader", tty_reader, "tty2");
    printf("\n");
    printf("End Testcase\n");
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
    iset(true);
    while (1)
        ;
}

static Context* os_trap(Event ev, Context* ctx)
{
    Context* next = NULL;
    for (trap_t* h = trap_head.next; h != NULL; h = h->next) {
        if (h->event == EVENT_NULL || h->event == ev.event) {
            Context* r = h->handler(ev, ctx);
            panic_on(r && next, "returning multiple contexts");
            if (r)
                next = r;
        }
    }
    panic_on(!next, "returning NULL context");
    //panic_on(sane_context(next), "returning to invalid context");
    return next;
}

static void os_on_irq(int seq, int event, handler_t handler)
{
    trap_t* cur_trap = &trap_head;
    while (cur_trap->next != NULL) {
        if (cur_trap->next->seq > seq)
            break;
        else
            cur_trap = cur_trap->next;
    }
    trap_t* new_trap = (trap_t*)pmm->alloc(sizeof(trap_t));
    new_trap->seq = seq;
    new_trap->event = event;
    new_trap->handler = handler;

    if (cur_trap->next != NULL)
        new_trap->next = cur_trap->next;
    else
        new_trap->next = NULL;
    cur_trap->next = new_trap;
}

MODULE_DEF(os) = {
    .init = os_init,
    .run = os_run,
    .trap = os_trap,
    .on_irq = os_on_irq,
};
