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
    int *a[10];
    for (int i = 0; i <= 7; i++)
    {
        a[i] = pmm->alloc(i MB);
    }
    pmm->alloc(128 MB);

    pmm->free(a[5]);

    pmm->stat();
    while (1)
        ;
}

MODULE_DEF(os) = {
    .init = os_init,
    .run = os_run,
};
