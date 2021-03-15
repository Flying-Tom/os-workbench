#include <game.h>

#define SIDE 8

struct point player;

int randint(int l, int r)
{
    return l + (rand() & 0x7fffffff) % (r - l + 1);
}

void resetgame()
{
    memset(coin_rec, 0, sizeof(coin_rec));
    player.x = randint(0, loc_x);
    player.y = randint(0, loc_y);
    generate_coin();
}

void generate_coin()
{
    int n = randint(0, 2);
    for (int i = 1; i <= n; i++)
    {
        int coin_x = randint(0, loc_x), coin_y = randint(0, loc_y);
        coin_rec[coin_x][coin_y] = 1;
        printf("new coin location:(%d,%d)\n", coin_x, coin_y);
    }
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
    srand(114514);
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
