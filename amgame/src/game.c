#include <game.h>

#define SIDE 8

struct Snake
{
    int x, y;
} snake;

void delay(uint64_t z)
{
    z = z * 10000;
    while (z--)
        ;
}

void generate_state()
{
    snake.x = rand() % 64;
    snake.y = rand() % 32;
}

void update_screen()
{
    draw_snake(snake.x, snake.y, 0xffffff);
    draw_snake(snake.x, snake.y, 0x0);
}

// Operating system is a C program!
int main(const char *args)
{
    ioe_init();

    puts("mainargs = \"");
    puts(args); // make run mainargs=xxx
    puts("\"\n");

    //splash();

    puts("Press any key to see its key code...\n");
    while (1)
    {
        exit_check();
        print_key();
        generate_state();
        //printf("snake.x:%d snake.y:%d\n", snake.x, snake.y);
        update_screen();
        delay(1000000000);
    }
    return 0;
}
