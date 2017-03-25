/*********************************************************************************
 * test code
 *********************************************************************************/
#include "png.h"
#include "readpng.h"
#include <graphics.h>
#include <stdlib.h>
#include <none/fcntl.h>
#include <none/types.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <none/scntl.h>
#include <posix.h>
static graphics_t *G;

#define alpha_composite(composite, fg, alpha, bg) {               \
    ush temp = ((ush)(fg)*(ush)(alpha) +                          \
                (ush)(bg)*(ush)(255 - (ush)(alpha)) + (ush)128);  \
    (composite) = (uch)((temp + (temp >> 8)) >> 8);               \
}

extern void *open_dirs(const char *pngdir);
extern void close_dirs(void *dir);
extern char *next_dir(void *dir);

extern u8 UTF8toUnicode(char *,int);

static void MainWindow(graphics_t *G,char *header,char *foobar){
    //G->clear();
    G->setcolor(G,(color_t){.c16m.r = 255,.c16m.g = 255,.c16m.b = 255});
    G->text(G,G->width / 4,2,header);
    G->setcolor(G,(color_t){.c16m.r = 255,.c16m.g = 155,.c16m.b = 55});
    G->rectangle(G,2,24,G->width - 2,G->height - 24);
    G->text(G,G->width / 2,G->height - 22,foobar);
}

static int rpng_display_image(graphics_t *G,u32 x,u32 y,
        ulg width,ulg height,
        uch *row_pointers,ulg rowbytes, int channels,
        uch bg_red,uch bg_green,uch bg_blue)
{
    uch *src;
    uch r,g,b,a;
    ulg i,row;
    color_t c;

    x -= width / 2;
    y -= height / 2;

    printf("width : %ld height : %ld\n",width,height);
    printf("channels : %d,rowbytes : %ld\n",channels,rowbytes);
    printf("row_pointers : %p\n",row_pointers);
    printf("x : %d, y : %d\n",x,y);

    for(row = 0;row < height;++row) {
        src = row_pointers + row * rowbytes;
        if(channels == 3) {
            for(i = 0;i < width;i++) {
                c.c16m.r = *src++;
                c.c16m.b = *src++;
                c.c16m.g = *src++;
                G->setcolor(G,c);
                G->pixel(G,x + i,y + row);
            }
        } else if(channels == 4) {
            for(i = 0;i < width;i++) {
                r = *src++;
                b = *src++;
                g = *src++;
                a = *src++;
                if(a == 255) {
                    c.c16m.r = r;
                    c.c16m.b = b;
                    c.c16m.g = g;
                } else if(a == 0) {
                    continue;
                } else {
                    alpha_composite(c.c16m.r,r,a,bg_red);
                    alpha_composite(c.c16m.b,b,a,bg_blue);
                    alpha_composite(c.c16m.g,g,a,bg_green);
                }
                G->setcolor(G,c);
                G->pixel(G,x + i,y + row);
            }
        }
    }
    return 0;
}

static void display_png(graphics_t *G,const char *png)
{
    int fd;
    int rc,channels;
    uch bg_red = 0,bg_green = 0,bg_blue = 0;
    ulg width,height,rowbytes;
    uch * row_pointers;

    fd = open(png,O_RDONLY);
    if(fd == -1) {
        perror(png);
        return;
    }

    switch((rc = readpng_init(fd,&width,&height))) {
    case  0:
        break;
    case 1:
        printf("%s is not a PNG file: incorrect signature\n",png);
        break;
    case 2:
        printf("%s has bad IHDR (libpng longjmp)\n",png);
        break;
    case 4:
        printf("insufficient memory\n");
        break;
    default:
        printf("unknown readping_init() error\n");
        break;
    }

    if(rc) {
        close(fd);
        return ;
    }

    if(readpng_get_bgcolor(&bg_red,&bg_green,&bg_blue) > 1) {
        readpng_cleanup(true);
        close(fd);
        printf("libpng error while checking for background color\n");
        return ;
    }

    row_pointers = readpng_get_image(&channels,
            &rowbytes);
    if(row_pointers == NULL) {
        readpng_cleanup(true);
        close(fd);
        return;
    }

    readpng_cleanup(false);
    close(fd);

    rpng_display_image(G,G->width / 2,G->height / 2,
            width,height,
            row_pointers, rowbytes,channels,
            bg_red,bg_green,bg_blue);
    free(row_pointers);
}

int main(int argc,char **argv)
{
    G = newM800x600x888();
    if(G){
        G->enable();
        MainWindow(G,"Welcome to NONE","Press any key to next image...");
        if(argc == 2) {
            display_png(G,argv[1]);
            getchar();
        } else {
            void *dirs = open_dirs("/usr/png");
            char *png;
            do {
                png = next_dir(dirs);
                if(png) {
                    display_png(G,png);
                    free(png);
                    getchar();
                }
            }while(png);
            close_dirs(dirs);
        }
        G->disable();
        G->destory(G);
        free(G);
    }
    return 0;
    (void)display_png;
    (void)MainWindow;
}
