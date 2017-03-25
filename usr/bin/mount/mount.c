#include <stdio.h>
#include <stdlib.h>
#include <sys/inter.h>
#include <none/fcntl.h>
#include <errno.h>

int main(int argc,char **argv) {
    int fs,fd;
    if(argc != 3)
        return -EINVAL;
    fs = open(argv[1],0);
    if(fs < 0)
        return fs;
    fd = open(argv[2],0);
    if(fd < 0) {
        close(fs);
        return fd;
    }
    return run(fd,FIF_MOUNT,fs,0,0);
}
