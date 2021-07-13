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

    printf("Tests done!\n");
    /*
    while (1)
        ;
    */
}

static Context* os_trap(Event ev, Context* context)
{
    Context* ret = NULL;
    return ret;
}

static void os_on_irq(int seq, int event, handler_t handler)
{
}

MODULE_DEF(os) = {
    .init = os_init,
    .run = os_run,
    .trap = os_trap,
    .on_irq = os_on_irq,
};
