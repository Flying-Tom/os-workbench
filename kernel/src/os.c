#include <common.h>
#include <devices.h>

//#define DEBUG_LOCAL

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
static void tty_reader(void* arg)
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

    printf("Start Testcase\n");
    /*
    kmt->sem_init(&empty, "empty", 5); // 缓冲区大小为 5
    kmt->sem_init(&fill, "fill", 0);
    for (int i = 0; i < 4; i++) // 4 个生产者
        kmt->create(pmm->alloc(sizeof(task_t)), "producer", producer, NULL);
    for (int i = 0; i < 5; i++) // 5 个消费者
        kmt->create(pmm->alloc(sizeof(task_t)), "consumer", consumer, NULL);
    */

    kmt->create(pmm->alloc(sizeof(task_t)), "tty_reader", tty_reader, "tty1");
    kmt->create(pmm->alloc(sizeof(task_t)), "tty_reader", tty_reader, "tty2");
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
    //iset(true);
    while (1)
        ;
}

static Context* os_trap(Event ev, Context* context)
{
    printf("Enter os_trap\n");
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
    //Log("trap_handlers[%d][%d] is available\n", seq, cnt);
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
