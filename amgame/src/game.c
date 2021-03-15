#include <game.h>

#define SIDE 8

struct point player;

int randint(int l, int r)
{
    return l + (rand() & 0x7fffffff) % (r - l + 1);
}

void resetgame()
{
    player.x = randint(0, loc_x);
    player.y = randint(0, loc_y);
    generate_coin();
}

void generate_coin()
{
    int n = randint(1, 1);
    coin_rec[randint(0, loc_x)][randint(0, loc_y)] = 1;
}

void bonus_check()
{
}

// Operating system is a C program!
int main(const char *args)
{
    ioe_init();

    puts("mainargs = \"");
    puts(args); // make run mainargs=xxx
    puts("\"\n");

    /*
    splash();
    puts("Press any key to see its key code...\n");
    */
    get_screen_info();

    resetgame();

    while (1)
    {
        render();
        key_process();
        //print_key();
        //generate_state();
    }
    return 0;
}
