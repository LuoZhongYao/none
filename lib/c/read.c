#include <sys/inter.h> 
#include <none/if.h>
#include <string.h>
#include <errno.h>

ssize_t read(int fd,void *buffer,int count){
    ssize_t len;
    void *buff = _push(NULL,count);
    if(0 < (len = run(fd,IF_READ,buff,count,0))){
        memcpy(buffer,buff,len);
    } else {
        errno = -len;
        len = -1;
    }
    _pop(buff);
    return len;
}
