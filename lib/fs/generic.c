#include <fs/fs.h>
#include <none/fcntl.h>
#include <none/scntl.h>
#include <sys/inter.h>
#include <errno.h>

int printk();
void generic_seek(object_t caller,int whence,off_t offset) {
    struct file *file = self_private();
    switch(whence) {
    case SEEK_SET: file->offset = offset; break;
    case SEEK_CUR: file->offset += offset; break;
    case SEEK_END: file->offset = offset + file->inode->i_size;break;
    }
    ret(caller,file->offset);
}

void generic_umount(object_t caller) {
    ret(caller,-ENOSYS);
}

void generic_mount(object_t caller,object_t mnt,umode_t mode) {
    struct file *file = self_private();
    struct inode *inode = file->inode;
    inode->i_mode = S_IFMNT | (mode & 0777);
    inode->i_rdev = mnt;
    hook(FIF_UMOUNT,generic_umount);
    ret(caller,OK);
}
