#include <game.h>

#define SIDE 8

struct Snake
{
    int x, y;
} snake;

void delay(int z)
{
    for (; z > 0; z--)
        ;
}

void generate_state()
{
    snake.x = rand() % 64;
    snake.y = rand() % 32;
}

void update_screen()
{
    draw_snake(snake.x, snake.y);
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
        update_screen();
        delay(10000000);
    }
    return 0;
}
