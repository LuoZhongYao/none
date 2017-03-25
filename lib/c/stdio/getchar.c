#include    <stdio.h>
#include <stdlib.h>
#include    <sys/inter.h>

extern int getchar(void){
    char ch;
    read(STDIN_FILENO,&ch,1);
    return ch;
}
