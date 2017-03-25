#include <stdlib.h>
#include <sys/inter.h>
#include <stdio.h>
#include <none/fcntl.h>
#include <string.h>
#define next_entry(x)   ((void*)x) + 32
struct dir_entry {
    u16 inode;
    char name[0];
};

struct dirs{
    int fd;
    int offset;
    int blk;
    const char *dirname;
    void *dir;
};

void *open_dirs(const char *dir)
{
    struct dirs *dirs = malloc(sizeof(dirs));
    dirs->dir = malloc(1024);
    dirs->blk = 0;
    dirs->offset = 0;
    dirs->dirname = strdup(dir);
    dirs->fd = open(dir,O_RDONLY);
    read(dirs->fd,dirs->dir,1024);
    return dirs;
}

void close_dirs(void *_dirs)
{
    struct dirs *dirs = _dirs;
    if(dirs) {
        if(dirs->dir)
            free(dirs->dir);
        if(dirs->dirname)
            free((void*)dirs->dirname);
        free(dirs);
    }
}

char *next_dir(void *_dirs)
{
    struct dirs *dirs = _dirs;
    struct dir_entry *dir;
    char *name = NULL;
    int nlen;
    if(dirs && dirs->offset >= 0) {
        do {
            dir = dirs->dir + dirs->offset;
            dirs->offset += 32;
        }while((!strcmp("..",dir->name) || !strcmp(".",dir->name)));
        if(strlen(dir->name)) {
            nlen = strlen(dirs->dirname) + strlen(dir->name) + 2;
            name = malloc(nlen);
            if(name) {
                memset(name,0,nlen);
                strcpy(name,dirs->dirname);
                strcat(name,"/");
                strncat(name,dir->name,30);
                if(dirs->offset >= 1024) {
                    dirs->offset = 0;
                    dirs->blk++;
                    if(0 > read(dirs->fd,dirs->dir,dirs->blk * 1024)){
                        dirs->offset = -1;
                    }
                }
            }
        }
    }
    return name;
}
