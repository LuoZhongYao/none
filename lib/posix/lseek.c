#include <sys/inter.h>
#include <none/fcntl.h>

off_t lseek(object_t o,off_t offset,int whence){
    return run(o,FIF_SEEK,whence,offset,0);
};
