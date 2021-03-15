#include <am.h>
#include <amdev.h>
#include <klib.h>
#include <klib-macros.h>

/* video.c */
void splash();
void draw_point(int x, int y, int color);
void render();

/* keyboard.c */
void print_key();
void exit_check();
void direction_control();

static inline void puts(const char *s)
{
    for (; *s; s++)
        putch(*s);
}
