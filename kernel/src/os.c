#include <common.h>

//#define SMOKE
#define NORMAL
//#define STRESSED

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
    printf("Tests Start!\n");
#ifdef SMOKE

    /*
    int *a = NULL;
    for (int i = 1; i <= 50; i++)
    {
        a = pmm->alloc(1);
        printf("a:%p\n", a);
        pmm->free(a);
    }
    */
    /*
    for (int i = 1; i < 4; i++)
    {
        size_t rand_m = 4;
        pmm->alloc(rand_m);
        printf("%d: Alloc %d success\n", i, rand_m);
    }
    */
#endif

#ifdef NORMAL

    /*
    for (int i = 1; i < 500000; i++)
    {
        //printf("Alloc\n");
        size_t rand_m = rand() % 128 + 1;
        printf("%p\n", pmm->alloc(rand_m));
        //printf("%d: Alloc %d success\n", i, rand_m);
    }
    */

    for (int i = 1; i < 10; i++)
    {
        size_t m = 2 MB;
        printf("%p\n", pmm->alloc(m));
        //printf("%d: Alloc %d success\n", i, rand_m KB);
    }

#endif
#ifdef STRESSED
    for (int i = 0; i < 1000; i++)
    {
        pmm->alloc(rand() % 32 MB + 1);
        //printf("Alloc %d success\n", 16 KB);
    }
#endif

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
