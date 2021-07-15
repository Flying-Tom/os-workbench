#include <common.h>

//#define SMOKE
#define NORMAL
//#define STRESSED

static void os_init()
{
    pmm->init();
    kmt->init();
}

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

    printf("Tests done!\n");

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
    //assert(0);
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
