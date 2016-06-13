#include "minix_fs.h"
#include <none/fcntl.h>
#include <none/scntl.h>

static struct super_block *super;

object_t mount_open(struct inode *inode,String name,umode_t mode) {
    LOGD("mount open(%d).\n",inode->i_rdev);
    return run2(inode->i_rdev,IF_OPEN,name,mode);
}

static void minix_close(object_t caller) {
    struct file *file = self()->private_data;
    file->cnt--;
    ret(caller,OK);
    if(file->cnt <= 0) {
        kfree(self()->private_data);
        self()->private_data = NULL;
        run0(MM_PID,MIF_CLOSE);
    }
}

static void minix_open(object_t caller,String name __unused,umode_t mode __unused) {
    struct file *file = self()->private_data;
    LOGD("double open(%s).\n",name);
    file->cnt++;
    ret(caller,self()->id);
}

object_t normal_open(struct inode *inode,String name,int flag,umode_t mode __unused) {
    if(inode) {
        object_t id = fork();
        if(0 == id) {
            struct file *file = kalloc(sizeof(struct file));
            strcpy(self()->name,name);
            file->flags = flag;
            file->offset = 0;
            if(flag & O_APPEND)
                file->offset = inode->i_size;
            file->inode = inode;
            file->cnt = 1;
            self()->private_data = file;
            hook(FIF_OPEN,minix_open);
            hook(FIF_READ,minix_read);
            hook(FIF_WRITE,minix_write);
            hook(FIF_CLOSE,minix_close);
            hook(FIF_SEEK,generic_seek);
            hook(FIF_MOUNT,generic_mount);
            workloop();
        } else {
            return id;
        }
    }
    return -ENFILE;
}

static void rootfs_open(object_t caller,void *pathname,int flag,
        umode_t mode) {
    int res;
    struct inode *inode;
    res = open_namei(super->s_root,pathname,flag,mode,&inode);
    if(res) {
        LOGD("%d opoen_namei(%s,%o,%o,%p).\n",
                res,pathname,flag,mode,&inode);
        ret(caller,res);
    } else {
        ret(caller,normal_open(inode,pathname,flag,mode));
    }
}

static void rootfs_stat(object_t caller,void *pathname,struct stat *stat_buf)
{
    int res;
    struct inode *inode;
    res = open_namei(super->s_root,pathname,O_RDONLY,0666,&inode);
    if(!res && stat_buf) {
        stat_buf->st_dev = inode->i_rdev;
        stat_buf->st_gid = inode->i_gid;
        stat_buf->st_ino = inode->i_ino;
        stat_buf->st_uid = inode->i_uid;
        stat_buf->st_mode = inode->i_mode;
        stat_buf->st_size = inode->i_size;
        stat_buf->st_atime = inode->i_atime.tv_sec;
        stat_buf->st_ctime = inode->i_ctime.tv_sec;
        stat_buf->st_mtime = inode->i_mtime.tv_sec;
        stat_buf->st_nlink = inode->i_nlinks;
    }
    ret(caller,res);
}

static void rootfs_mkdir(object_t caller,void *buffer,umode_t mode) {
    int res = minix_mkdir(super->s_root,buffer,mode);
    if(res) {
        LOGD("%d minix_mkdir(%s,%o).\n",res,buffer,mode);
        ret(caller,res);
    } else
        ret(caller,res);
}

static void rootfs_rmdir(object_t caller,void *name) {
    int res = minix_rmdir(super->s_root,name);
    if(res) {
        LOGD("%d minix_rmdir(%s).\n",res,name);
        ret(caller,res);
    } else {
        ret(caller,res);
    }
}


static void info_inode(struct inode *inode) {
    LOGI(".......inode info....\n");
    LOGI("ino    : %u\n",inode->i_ino);
    LOGI("i_size : %u\n",inode->i_size);
    LOGI("i_mode : %u\n",inode->i_mode);
}
static void info_super(struct super_block *sb) {
    struct minix_sb_info *sbi = sb_info(sb);
    LOGI("..............super block...............\n");
    LOGI("version            : %s\n",
            sbi->s_version == MINIX_V1 ? "minix v1" :
            sbi->s_version == MINIX_V2 ? "minix v2" :
            "unkonw");
    LOGI("device             : %u\n",super->s_dev);
    LOGI("ninodes            : %u\n",sbi->s_ninodes);
    LOGI("nzones             : %u\n",sbi->s_nzones);
    LOGI("inode size         : %u\n",sbi->s_inosize);
    LOGI("dzones             : %u\n",sbi->s_dzones);
    LOGI("dir size           : %u\n",sbi->s_dirsize);
    LOGI("name length        : %u\n",sbi->s_namelen);
    LOGI("imap blocks        : %u\n",sbi->s_imap_blocks);
    LOGI("zmap blocks        : %u\n",sbi->s_zmap_blocks);
}

static void info_bitmap(struct super_block *sb,unsigned who,unsigned bnr)
{
    struct minix_sb_info *sbi = sb_info(sb);
    bnr *= 0x180;
    unsigned long blk =    (bnr + 1023) / 1024;
    unsigned long offset = bnr % 1024;
    u8 *map = who ? sbi->s_imap[blk] : sbi->s_zmap[blk];
    map += offset;
    unsigned i;
    printk("------------------BIT MAP (%p)-------------------",map);
    for(i = 0;i < 0x180;i++) {
        if(!(i % 16))
            printk("\n%04x : ",i + bnr);
        printk("%02x ",map[i]);
    }
}

static void rootfs_debug(object_t caller,unsigned long cmd,unsigned a1,unsigned a2)
{
    switch(cmd) {
    case 0:
        info_super(super);
        break;
    case 1:
        {
            struct inode *inode = minix_find_inode(super,a1);
            info_inode(inode);
        }
        break;
    case 2:
        info_bitmap(super,a1,a2);
        break;
    default:
        break;
    }
    ret(caller,OK);
}

static void rootfs_init(void){
    int error;
    super = minix_sget(RAMDISK_PID,&error);
    if(super == NULL) 
        panic("Don't read super block.\n");

    hook(FIF_OPEN ,rootfs_open);
    hook(FIF_STAT,rootfs_stat);
    hook(FIF_MKDIR,rootfs_mkdir);
    hook(FIF_RMDIR,rootfs_rmdir);
    hook(FIF_DEBUG,rootfs_debug);
}

int rootfs_main(void )
{
    rootfs_init();
    workloop();
    return 0;
}
