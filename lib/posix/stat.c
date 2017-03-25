#include <sys/inter.h>
#include <none/stat.h>
#include <string.h>
#include <errno.h>

int stat(const char *pathname,struct stat *stat_buf)
{
    int res;
    void *buff = _push(NULL,sizeof(struct stat));
    if(!stat_buf)
        return 0;

    res = run(ROOTFS_PID,pathname,buff,0,0);
    if(res) {
        errno = -res;
        res = -1;
    }
    memcpy(stat_buf,buff,sizeof(struct stat));
    _pop(buff);
    return res;
}
