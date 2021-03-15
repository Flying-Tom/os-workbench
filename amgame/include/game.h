#include <am.h>
#include <amdev.h>
#include <klib.h>
#include <klib-macros.h>

/* video.c */
int screen_w, screen_h;
void get_screen_info();
void splash();
void draw_point(int x, int y, int color);
void render();

/* keyboard.c */
void print_key();
void key_process();

/* game */
int loc_x, loc_y;

struct point
{
    int x, y;
};

static inline void puts(const char *s)
{
    for (; *s; s++)
        putch(*s);
}
