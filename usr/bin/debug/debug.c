#include <stdio.h>
#include <none/fcntl.h>
#include <sys/inter.h>
#include <string.h>
#include <stdlib.h>

#define debug(a,...) run(ROOTFS_PID,FIF_DEBUG,a,##__VA_ARGS__)

int main(int argc,char **argv) {
    if(argc < 2) {
        printf("Usage : debug subcommand [args]\n");
        return -1;
    }
    if(!strcmp(argv[1],"inode")) {
        return debug(1,atoi(argv[2]),0);
    } else if (!strcmp(argv[1],"super")) {
        return debug(0,0,0);
    } else if(!strcmp(argv[1],"zmap")) {
        return debug(2,0,atoi(argv[2]));
    } else if(!strcmp(argv[1],"imap")) {
        return debug(2,1,atoi(argv[2]));
    } else {
        printf("unkown subcommand %s.\n",argv[1]);
    }
    return -1;
}
