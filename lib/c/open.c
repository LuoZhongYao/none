#include <sys/inter.h> 
#include <none/if.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

object_t open(String path,int flag,...){
    object_t res;
    va_list arg;
    void *buff = _push(path,strlen(path) + 1);
    va_start(arg,flag);
    res = run(ROOTFS_PID,IF_OPEN,buff,flag,0);
    if(res < 0) {
        errno = -res;
        res = -1;
    }
    _pop(buff);
    return res;
}

int close(object_t obj){
    return run(obj,IF_CLOSE,0,0,0);
}
