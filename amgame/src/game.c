#include <game.h>

#define SIDE 8

struct point
{
    int x, y;
} p;

void generate_state()
{
    snake.x = rand() % 64;
    snake.y = rand() % 32;
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
        //draw_snake(snake.x, snake.y, 0xffffff);
        //generate_state();

        //update_screen();
    }
    return 0;
}
