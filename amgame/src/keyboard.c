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

void exit_check()
{
    AM_INPUT_KEYBRD_T event = {.keycode = AM_KEY_NONE};
    ioe_read(AM_INPUT_KEYBRD, &event);
    if (event.keycode == AM_KEY_ESCAPE && event.keydown)
        halt(0);
}

extern struct point p;

void direction_control()
{
    AM_INPUT_KEYBRD_T event = {.keycode = AM_KEY_NONE};
    ioe_read(AM_INPUT_KEYBRD, &event);
    while (event.keycode == AM_KEY_NONE)
        ;
    if (event.keydown)
    {
        switch (event.keycode)
        {
        case AM_KEY_W:
            p.x--;
            break;
        case AM_KEY_A:
            break;
        case AM_KEY_D:
            break;
        case AM_KEY_S:
            p.x++;
            break;

        default:
            break;
        }
    }
}
