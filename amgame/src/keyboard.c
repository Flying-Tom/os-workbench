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

extern struct point p;

void key_process()
{
    AM_INPUT_KEYBRD_T event = {.keycode = AM_KEY_NONE};
    ioe_read(AM_INPUT_KEYBRD, &event);

    if (event.keycode != AM_KEY_NONE && event.keydown)
    {
        switch (event.keycode)
        {
        case AM_KEY_W:
            p.y = p.y > 0 ? p.y - 1 : p.y;
            break;
        case AM_KEY_A:
            p.x = p.x > 0 ? p.x - 1 : p.x;
            break;
        case AM_KEY_D:
            p.x++;
            break;
        case AM_KEY_S:
            p.y++;
            break;
        case AM_KEY_ESCAPE:
            halt(0);
            break;

        default:
            break;
        }
    }
}
