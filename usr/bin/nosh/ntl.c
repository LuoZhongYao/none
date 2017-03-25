#include "util.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
void *realloc(void *buffer,size_t size){
    void *new;
    new = malloc(size);
    if(new){
        memcpy(new,buffer,size);
    }
    free(buffer);
    return new;
}

static inline int parse(char *buffer,char **argv,int len){
    char *pos = buffer;
    int argc = 0;
    argv[0] = buffer;
    while(*pos){
        switch(*pos){
            case ' ' : 
                *pos = 0;
                while(*(++pos) && *pos == ' ');
                argc++;
                if(argc >= len) return argc;
                argv[argc] = pos;
                break;
        }
        pos++;
    }
    if(pos != buffer) argc++;
    return argc;
}
#include <sys/inter.h>
#ifdef  wait
#undef  wait
#define wait(x) run(MM_PID,15,0,0,0)
#endif
int system(char *cmd) {
    object_t o;
    char *argv[10];
    int  argc = 0;
    argc = parse(cmd,argv,10);
    o = fork();
    if(0 == o){
        exec(argv[0],argc,argv);
    } else if(0 < o) {
        wait(o);
    } else {
        printf("Error\n");
        return -1;
    }
    return 0;
}

int atoi(const char *string) {
    int number = 0;
    while(*string){
        if(*string >= '0' && *string <= '9'){
            number = number * 10 + *string - '0';
        } else {
            break;
        }
        string++;
    }
    return number;
}

int isatty(int fd)
{
    return 1;
}
