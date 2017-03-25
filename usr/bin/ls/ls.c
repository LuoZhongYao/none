/*******************************************************************************
 *  巫师:
 *      lzy
 *  日辰:
 *      Wed Dec  4 19:58:53 CST 2013
 *  巫术:
 *      只能打印当前目录的ls
 * *****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/inter.h>

#define next_entry(x)   ((void*)x) + 32;
struct dir_entry{
    u16     inode;
    char    name[0];
};

int main(int argc,char **argv){
    void *bb;
    int i,j;
    struct dir_entry *dir;
    bb = malloc(1024);
    for(i = 1;i < argc;i++){
        int curr = open(argv[i],0);
        ssize_t len;
        if(curr != ERROR){
            while(0 < (len = read(curr,bb,1024))){
                dir = bb;
                for(j = 0;j < (len / 32);j++){
                    if(dir->inode && strcmp(".",dir->name) && strcmp("..",dir->name))
                        printf("%10s\n",dir->name);
                    dir = next_entry(dir);
                }
            }
            close(curr);
        }
    }
    free(bb);
    return 0;
}
