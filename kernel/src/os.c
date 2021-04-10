#include <common.h>

static void os_init()
{
    pmm->init();
}

static void os_run()
{
    for (const char *s = "Hello World from CPU #*\n"; *s; s++)
    {
        putch(*s == '*' ? '0' + cpu_current() : *s);
    }
    pmm->alloc(0 MB);
    pmm->alloc(1 MB);
    pmm->alloc(2 MB);
    pmm->alloc(3 MB);
    pmm->alloc(4 MB);
    pmm->alloc(5 MB);
    pmm->alloc(6 MB);
    pmm->alloc(7 MB);
    pmm->alloc(128 MB);
    kstat();
    while (1)
        ;
}

MODULE_DEF(os) = {
    .init = os_init,
    .run = os_run,
};
