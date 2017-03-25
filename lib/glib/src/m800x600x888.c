#include "graphics.h"
#include "vbe.h"
#include <none/scntl.h>
#include <sys/inter.h>
#include <string.h>
#define video ((uint8_t *)0xe0000000)
#define HEIGHT  600
#define WIDTH   800

static void pixel(graphics_t *thiz,int x,int y)
{
    unsigned long offset = 0;
    if(x < WIDTH && y < HEIGHT){
        offset = (y * 800 + x) * 3;
        (video + offset)[0] = thiz->color.c16m.b;
        (video + offset)[1] = thiz->color.c16m.g;
        (video + offset)[2] = thiz->color.c16m.r;
    }
}

static color_t getpixel(graphics_t *thiz,int x,int y)
{
    unsigned long offset = 0;
    color_t c = {.c24b = 0};
    if(x < WIDTH && y < HEIGHT){
        offset = (y * 800 + x) * 3;
        c.c16m.b = (video + offset)[0];
        c.c16m.g = (video + offset)[1];
        c.c16m.r = (video + offset)[2];
    }
    return c;
}

static void fill(graphics_t *G)
{
    for(int y = 0;y < HEIGHT;y++) {
        for(int x = 0;x < WIDTH;x++) {
            G->pixel(G,x,y);
        }
    }
}

static void enable(void)
{
    graphics_install_bochs(WIDTH,HEIGHT);
}

static void disable(void)
{
    graphics_uninstall_bochs();
}

static void clear(void)
{
    memset(video,0,1 << 24);
}

graphics_t *newM800x600x888(void)
{
    graphics_t *g;
    g = newGraphics();
    if(g){
        g->pixel    = pixel;
        g->getpixel = getpixel;
        g->enable   = enable;
        g->fill     = fill;
        g->disable  = disable;
        g->clear    = clear;
        g->height   = HEIGHT;
        g->width    = WIDTH;
    }
    return g;
}
