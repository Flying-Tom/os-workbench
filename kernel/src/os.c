#include <common.h>
#define DEBUG

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
    //int *a[100];
    for (int i = 1; i < 100; i++)
    {
        pmm->alloc(rand() % 128 + 1);
        //printf("Alloc %d success\n", 16 KB);
    }
    for (int i = 0; i < 4; i++)
    {
        pmm->alloc(rand() % 8 MB + 1);
        //printf("Alloc %d success\n", 16 KB);
    }
    pmm->alloc(430);

    //pmm->stat();
    /*
    pmm->free(a[7]);
    pmm->free(a[5]);
    pmm->free(a[4]);

    pmm->free(a[6]);
    */
    //pmm->stat();
    printf("Tests done!\n");
    while (1)
        ;
}

MODULE_DEF(os) = {
    .init = os_init,
    .run = os_run,
};
