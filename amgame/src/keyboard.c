#include <game.h>

#define KEYNAME(key) \
    [AM_KEY_##key] = #key,
static const char *key_names[] = {
    AM_KEYS(KEYNAME)};

void print_key()
{
    AM_INPUT_KEYBRD_T event = {.keycode = AM_KEY_NONE};
    ioe_read(AM_INPUT_KEYBRD, &event);
    if (event.keycode != AM_KEY_NONE && event.keydown)
    {
        puts("Key pressed: ");
        puts(key_names[event.keycode]);
        puts("\n");
    }
}

void key_process()
{
    AM_INPUT_KEYBRD_T event = {.keycode = AM_KEY_NONE};
    ioe_read(AM_INPUT_KEYBRD, &event);

    if (event.keycode != AM_KEY_NONE && event.keydown)
    {
        switch (event.keycode)
        {
        case AM_KEY_W:
            player.y = player.y > 0 ? player.y - 1 : player.y;
            break;
        case AM_KEY_A:
            player.x = player.x > 0 ? player.x - 1 : player.x;
            break;
        case AM_KEY_D:
            player.x = player.x < loc_x ? player.x + 1 : player.x;
            break;
        case AM_KEY_S:
            player.y = player.y < loc_y ? player.y + 1 : player.y;
            break;
        case AM_KEY_ESCAPE:
            halt(0);
            break;

            if (coin_rec[player.x][player.y])
            {
                coin_rec[player.x][player.y] = 0;
                generate_coin();
            }
        default:
            break;
        }
    }
}
