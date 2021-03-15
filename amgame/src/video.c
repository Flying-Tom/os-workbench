#include <game.h>

#define SIDE 8

void get_screen_info()
{
    AM_GPU_CONFIG_T info = {0};
    ioe_read(AM_GPU_CONFIG, &info);
    screen_w = info.width;
    screen_h = info.height;
}

static void draw_tile(int x, int y, int w, int h, uint32_t color)
{
    uint32_t pixels[w * h]; // WARNING: large stack-allocated memory
    AM_GPU_FBDRAW_T event = {
        .x = x,
        .y = y,
        .w = w,
        .h = h,
        .sync = 1,
        .pixels = pixels,
    };
    for (int i = 0; i < w * h; i++)
    {
        pixels[i] = color;
    }
    ioe_write(AM_GPU_FBDRAW, &event);
}

void splash()
{
    get_screen_info();
    for (int x = 0; x * SIDE <= screen_w; x++)
    {
        for (int y = 0; y * SIDE <= screen_h; y++)
        {
            if ((x & 1) ^ (y & 1))
            {
                draw_tile(x * SIDE, y * SIDE, SIDE, SIDE, 0xffffff); // white
            }
        }
    }
}

extern struct point p;

void render()
{
    for (int x = 0; x * SIDE <= screen_w; x++)
    {
        for (int y = 0; y * SIDE <= screen_h; y++)
        {
            if (x == p.x && y == p.y)
                draw_tile(x * SIDE, y * SIDE, SIDE, SIDE, 0xffffff); // white
            else
                draw_tile(x * SIDE, y * SIDE, SIDE, SIDE, 0x0);
        }
    }
}

void draw_point(int x, int y, int color)
{
    draw_tile(x * SIDE, y * SIDE, SIDE, SIDE, color);
}
