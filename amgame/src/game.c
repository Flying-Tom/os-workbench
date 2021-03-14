#include <game.h>

#define SIDE 8

struct Snake
{
    int x, y;
} snake;

void generate_state()
{
    snake.x = rand() % 64;
    snake.y = rand() % 32;
}

void update_screen()
{
    draw_tile(snake.x * SIDE, snake.y * SIDE, SIDE, SIDE, 0xffffff);
}

// Operating system is a C program!
int main(const char *args)
{
    ioe_init();

    puts("mainargs = \"");
    puts(args); // make run mainargs=xxx
    puts("\"\n");

    splash();

    puts("Press any key to see its key code...\n");
    while (1)
    {
        exit_check();
        print_key();
        generate_state();
        update_screen();
    }
    return 0;
}
