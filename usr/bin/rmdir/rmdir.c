#include <none/fcntl.h>
#include <sys/inter.h>

int main(int argc,char **argv) {
    int res;
    if(argc < 2)
        return -1;
    for(int i = 1;i < argc;i++) {
        res = run2(ROOTFS_PID,FIF_RMDIR,argv[i],0666);
        if(res < 0)
            return res;
    }
    return -1;
}
