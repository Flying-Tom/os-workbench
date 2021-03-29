#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "co-test.h"

int g_count = 0;

static void add_count()
{
    g_count++;
}

static int get_count()
{
    return g_count;
}

static void work_loop(void *arg)
{
    const char *s = (const char *)arg;
    for (int i = 0; i < 100; ++i)
    {
        printf("%s%d  ", s, get_count());
        add_count();
        co_yield();
    }
}

static void work(void *arg)
{
    work_loop(arg);
}

static void test_1()
{
    struct co *thd1 = co_start("thread-1", work, "X");
    struct co *thd2 = co_start("thread-2", work, "Y");

    co_wait(thd1);
    co_wait(thd2);

    //    printf("\n");
}

// -----------------------------------------------

static int g_running = 1;

static void do_produce(Queue *queue)
{
    assert(!q_is_full(queue));
    Item *item = (Item *)malloc(sizeof(Item));
    if (!item)
    {
        fprintf(stderr, "New item failure\n");
        return;
    }
    item->data = (char *)malloc(10);
    if (!item->data)
    {
        fprintf(stderr, "New data failure\n");
        free(item);
        return;
    }
    memset(item->data, 0, 10);
    sprintf(item->data, "libco-%d", g_count++);
    q_push(queue, item);
}

static void producer(void *arg)
{
    Queue *queue = (Queue *)arg;
    for (int i = 0; i < 100;)
    {
        if (!q_is_full(queue))
        {
            // co_yield();
            do_produce(queue);
            i += 1;
        }
        co_yield();
    }
}

static void do_consume(Queue *queue)
{
    assert(!q_is_empty(queue));

    Item *item = q_pop(queue);
    if (item)
    {
        printf("%s  ", (char *)item->data);
        free(item->data);
        free(item);
    }
}

static void consumer(void *arg)
{
    Queue *queue = (Queue *)arg;
    while (g_running)
    {
        if (!q_is_empty(queue))
        {
            do_consume(queue);
        }
        co_yield();
    }
}

static void test_2()
{

    Queue *queue = q_new();

    struct co *thd1 = co_start("producer-1", producer, queue);
    struct co *thd2 = co_start("producer-2", producer, queue);
    struct co *thd3 = co_start("consumer-1", consumer, queue);
    struct co *thd4 = co_start("consumer-2", consumer, queue);

    co_wait(thd1);
    co_wait(thd2);

    g_running = 0;

    co_wait(thd3);
    co_wait(thd4);

    while (!q_is_empty(queue))
    {
        do_consume(queue);
    }

    q_free(queue);
}

///////////

static void do_test3(void *arg)
{
    for (int i = 0; i < 100; i++)
    {
        printf("%s %d %d\n", arg, i, get_count());
        add_count();
        if (rand() % 2 == 0)
            co_yield();
    }
}

struct co *thd[128];
char tmp[128][2];
static void test_3()
{
    for (int j = 0; j < 100; j++)
    {
        for (int i = 1; i <= 127; i++)
        {
            tmp[i][0] = i + '0';
            tmp[i][1] = '\0';
            printf("%s\n", tmp[i]);
            thd[i] = co_start(tmp[i], do_test3, tmp[i]);
        }

        for (int i = 127; i >= 1; i--)
        {
            co_wait(thd[i]);
        }
    }
    printf("success test 3\n");
}

////////////

int main()
{
    setbuf(stdout, NULL);
    /*
    printf("Test #1. Expect: (X|Y){0, 1, 2, ..., 199}\n");
    test_1();

    printf("\n\nTest #2. Expect: (libco-){200, 201, 202, ..., 399}\n");
    test_2();

    printf("\n\n");
    */
    do_test3("fuck");
    return 0;
}
