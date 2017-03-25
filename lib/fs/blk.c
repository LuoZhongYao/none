#include <fs/fs.h>
#include <none/util.h>

int blk_rw(const object_t dev,const int cmd,
        void *buff,off_t offset,blkcnt_t count){
    return run(dev,cmd,buff,count,offset);
}

int sb_bread(struct super_block *sb,void *buff,off_t offset) {
    return blk_readpage(sb->s_bdev,buff,offset);
}

int sb_bwrite(struct super_block *sb,void *buff,off_t offset) {
    return blk_writepage(sb->s_bdev,buff,offset);
}
