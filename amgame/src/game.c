#include <game.h>

#define SIDE 8

struct point p;

void generate_state()
{
    //snake.x = rand() % 64;
    //snake.y = rand() % 32;
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
    p.x = p.y = 5;
    get_screen_info();

    while (1)
    {
        print_key();
        key_process();
        render();
        //draw_snake(snake.x, snake.y, 0xffffff);
        //generate_state();

        //update_screen();
    }
    return 0;
}
