#include <sys/inter.h> 
#include <none/if.h>
#include <string.h>

ssize_t write(int fd,const void *buffer,size_t count){
    ssize_t len;
    void *buff = _push(buffer,count);
    len = run(fd,IF_WRITE,buff,count,0);
    _pop(buff);
    return len;
}
