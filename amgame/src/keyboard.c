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
        printf("fuck\n");
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
        case AM_KEY_ESCAPE:
            halt(0);
            break;

        default:
            break;
        }
    }
}
