#include <none/fcntl.h>
#include <sys/inter.h>
#include <stdio.h>
#include <errno.h>

int main(int argc,char **argv) {
    int res = -1;
    if(argc < 2)
        return -1;
    for(int i = 1;i < argc;i++) {
        res = run2(ROOTFS_PID,FIF_MKDIR,argv[i],0777);
        if(res < 0) {
            errno = -res;
            perror(argv[i]);
            return res;
        }
    }
    return res;
}
