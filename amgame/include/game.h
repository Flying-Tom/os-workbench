#include <am.h>
#include <amdev.h>
#include <klib.h>
#include <klib-macros.h>

void splash();
void print_key();
void exit_check();
void draw_snake(int, int);
void draw_apple();

static inline void puts(const char *s)
{
    for (; *s; s++)
        putch(*s);
}

