#include <stdlib.h>
#include <string.h>
#include "graphics.h"

#define SWAP(a,b) { a ^= b; b ^= a; a ^= b; }
#define ABS(x)  ((x) >= 0 ? (x) : -(x))
static void line(graphics_t *thiz,int x1,int y1,int x2,int y2){
    int dx = ABS(x2 - x1);
    int dy = ABS(y2 - y1);
    int yx = 0;

    if( dx < dy){
        SWAP(x1,y1);
        SWAP(x2,y2);
        SWAP(dx,dy);
        yx = 1;
    }
    int ix = (x2 - x1) > 0 ? 1 : -1;
    int iy = (y2 - y1) > 0 ? 1 : -1;
    int n2dy = dy << 1;
    int n2dydx = (dy - dx) << 1;
    int d = (dy << 1) - dx;
    if(yx){
        for(int cx = x1,cy = y1;cx != x2;cx += ix){
            if(d < 0){
                d += n2dy;
            }else{
                cy += iy;
                d += n2dydx;
            }
            thiz->pixel(thiz,cy,cx);
        }
    }else{
        for(int cx = x1,cy = y1;cx != x2;cx += ix){
            if(d < 0){
                d += n2dy;
            }else{
                cy += iy;
                d += n2dydx;
            }
            thiz->pixel(thiz,cx,cy);
        }
    }
    thiz->cur_pixel.x = x2;
    thiz->cur_pixel.y = y2;
}

static inline void circle8(graphics_t *thiz,int xc, int yc, int x, int y) {
    // 参数 c 为颜色值
    thiz->pixel(thiz, xc + x, yc + y);
    thiz->pixel(thiz, xc - x, yc + y);
    thiz->pixel(thiz, xc + x, yc - y);
    thiz->pixel(thiz, xc - x, yc - y);
    thiz->pixel(thiz, xc + y, yc + x);
    thiz->pixel(thiz, xc - y, yc + x);
    thiz->pixel(thiz, xc + y, yc - x);
    thiz->pixel(thiz, xc - y, yc - x);
}
 
//Bresenham's circle algorithm
static void circle(graphics_t *thiz,int xc, int yc, int r, int fill) {
    int x = 0, y = r, yi, d;
    d = 3 - (r << 1);

    if (fill) {
        while (x <= y) {
            for (yi = x; yi <= y; yi ++)
                circle8(thiz,xc, yc, x, yi);
            if (d < 0) {
                d = d + (x << 2) + 6;
            } else {
                d = d + ((x - y) << 2) + 10;
                y --;
            }
            x++;
        }
    } else {
        while (x <= y) {
            circle8(thiz,xc, yc, x, y);
            if (d < 0) {
                d = d + (x << 2) + 6;
            } else {
                d = d + ((x - y) << 2) + 10;
                y --;
            }
            x ++;
        }
    }
}

static void poly(graphics_t *thiz,int n,pixel_t *border){
    int nn;
    nn = n * 2;
    line(thiz,border[nn - 1].x,border[nn - 1].y,border[0].x,border[0].y);
    for(int i = 0;i < nn - 1;i++){
        line(thiz,border[i].x,border[i].y,border[i + 1].x,border[i + 1].y);
    }
}

static void rectangle(graphics_t *thiz,int x1,int y1,int x2,int y2){
    line(thiz,x1,y1,x2,y1);
    line(thiz,x2,y1,x2,y2);
    line(thiz,x1,y2,x2,y2);
    line(thiz,x1,y1,x1,y2);
}


static void setcolor(graphics_t *thiz,color_t color) {
    if(thiz)
        thiz->color = color;
}

#if 1
#include "font14.h"
static int putchar(graphics_t *thiz,int x,int y,const char ch){
    int offset;
    uint8_t map;
    offset = ch * 14;
    for(int i = 0;i < 14;i++){
        map = (vgafont14 + offset)[i];
        for(int j = 0;j < 8;j++){
            if((map >> (7 - j)) & 1)
                thiz->pixel(thiz,x + j,y + i);
        }
    }
    return 8;
}
#endif

#if 0
#include "font.h"
static int putu(graphics_t *thiz,int x,int y,uint16_t ch){
    struct{
        int x,y,xo,yo;
        int bitmap[0];
    }*data = (void*)(font.chars[ch]);
    int offset = 0;
    if(data){
        if(data->x <= 8)
            offset = 7;
        else if( data->x <= 16)
            offset = 15;
        else if(data->x <= 24)
            offset = 23;
        else 
            offset = 31;

        for(int i = 1;i <= data->y;i++){
            for(int j = 0;j < data->x;j++){
                if((data->bitmap[data->y - i] >> (offset - j)) & 1){
                    thiz->pixel(thiz,x + j + data->xo,y + (font.y - (i + data->yo)));
                }
            }
        }
        return data->x + 2;
    }
    return font.x;
}

#endif
uint16_t UTF8toUnicode(uint8_t *ch,int len);
static void text(graphics_t *thiz,int x,int y,const char *text){
    int len = strlen(text);
    uint16_t ch;
    (void)putchar;
    for(int i = 0;i < len;){
        ch = UTF8toUnicode((void*)text + i,len - i);
        if(ch == 0xffff){
            i++;
            continue;
        }
        if(ch < 0x80)
            i++;
        else if(ch < 0x0800)
            i += 2;
        else if(ch <= 0xffff)
            i += 3;
         x += putchar(thiz,x,y,ch);
         thiz->cur_pixel.x = x;
    }
}


static void moveto(graphics_t *thiz,int x,int y){
    if(x < thiz->width && y < thiz->height){
        thiz->cur_pixel.x = x;
        thiz->cur_pixel.y = y;
    }
}

static void destory(graphics_t *G __attribute__((unused)))
{

}

graphics_t *newGraphics(void){
    graphics_t *g;
    g = malloc(sizeof(*g));
    if(g){
        memset(g,0,sizeof(*g));
        g->setcolor = setcolor;
        g->line = line;
        g->circle = circle;
        g->poly = poly;
        g->rectangle = rectangle;
        g->text = text;
        g->moveto = moveto;
        g->destory = destory;
    }
    return g;
}
