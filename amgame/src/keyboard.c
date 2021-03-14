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
    if (event.keycode == AM_KEY_ESCAPE && event.keydown)
        _halt();
}
