#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc,char **args) {
    static char buf[1025];
    int fd;
    int size;
    if(argc != 2)
        return -EINVAL;

    fd = open(args[1],0);
    if(fd < 0)
        return fd;

    while((size = read(fd,buf,sizeof(buf) - 1)) > 0){
        buf[size] = 0;
        printf("%s",buf);
    }
    close(fd);
    return 0;
}
