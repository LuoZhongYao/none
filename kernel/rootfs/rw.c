#include "minix_fs.h"
#include <none/util.h>
static int v2_read(struct inode *inode,void *buffer,off_t offset,cnt_t count){
    u8 *blk;
    unsigned long  zone = offset / BLOCK_SIZE;
    cnt_t nrbytes = BLOCK_SIZE - (offset % BLOCK_SIZE);

    blk = kalloc(BLOCK_SIZE);
    if(!blk)
        return -ENOMEM;

    /*! ~~~~~~~~~~~~~~~~~~~ 第一步,拷贝开头不足一个区部分 ~~~~~~~~~~~~~~~~~~~~~~~ !*/
    count = MIN(inode->i_size - offset,count);
    nrbytes = MIN(count ,nrbytes);

    if(nrbytes){
        try(0 > ,inode_bread(inode,blk,zone),throw e_zone_rw);
        memcpy(buffer,blk + (offset % BLOCK_SIZE),nrbytes);
    }
    count -= nrbytes;
    zone++;
    /*! ~~~~~~~~~~~~~~~~~~~~~~~ 第二步,拷贝BLOCK_SIZE对齐的块 ~~~~~~~~~~~~~~~~~~ ~*/
    cnt_t n = count / BLOCK_SIZE;
    foreach(i,0,n){
        try(0 > ,inode_bread(inode,blk,zone),{
            nrbytes = -1;
            throw e_zone_rw;});
        memcpy(buffer + nrbytes,blk,count);
        count -= BLOCK_SIZE;
        nrbytes += BLOCK_SIZE;
        zone++;
    }
    /*! ~~~~~~~~~~~~~~~~~~~~~ 第三步,拷贝剩下的不足BLOCK_SIZE的块 ~~~~~~~~~~~~~ !*/
    if(count){
        try(0 > ,inode_bread(inode,blk,zone),throw e_zone_rw);
        memcpy(buffer + nrbytes,blk,count);
        nrbytes += count;
    }

    catch(e_zone_rw){ 
        kfree(blk);
        return nrbytes;
    }
}

static int v2_write(struct inode *inode,void *buffer,off_t offset,cnt_t count){
    u8 *blk;
    unsigned long  zone = offset / BLOCK_SIZE;
    cnt_t nrbytes = BLOCK_SIZE - (offset % BLOCK_SIZE);

    blk = kalloc(BLOCK_SIZE);
    if(!blk)
        return -ENOMEM;

    /*! ~~~~~~~~~~~~~~~~~~~ 第一步,拷贝开头不足一个区部分 ~~~~~~~~~~~~~~~~~~~~~~~ !*/
    //count = MIN(inode->i_size - offset,count);
    nrbytes = MIN(count ,nrbytes);
    if(nrbytes){
        try(0 > ,inode_bread(inode,blk,zone),throw e_zone_rw);
        memcpy(blk + (offset % BLOCK_SIZE),blk,nrbytes);
        try(0 > ,inode_bwrite(inode,blk,zone),throw e_zone_rw);
    }
    count -= nrbytes;
    zone++;
    /*! ~~~~~~~~~~~~~~~~~~~~~~~ 第二步,拷贝BLOCK_SIZE对齐的块 ~~~~~~~~~~~~~~~~~~ ~*/
    cnt_t n = count / BLOCK_SIZE;
    foreach(i,0,n){
        memcpy(blk,buffer + nrbytes,BLOCK_SIZE);
        try(0 > ,inode_bwrite(inode,blk,zone),throw e_zone_rw);
        count -= BLOCK_SIZE;
        nrbytes += BLOCK_SIZE;
        zone++;
    }
    /*! ~~~~~~~~~~~~~~~~~~~~~ 第三步,拷贝剩下的不足BLOCK_SIZE的块 ~~~~~~~~~~~~~ !*/
    if(count){
        try(0 >,inode_bread(inode,blk,zone),throw e_zone_rw);
        memcpy(blk,buffer + nrbytes,count);
        try(0 >,inode_bwrite(inode,blk,zone),throw e_zone_rw);
        nrbytes += nrbytes;
    }
    if(nrbytes + offset > inode->i_size)
        inode->i_size = nrbytes + offset;
    inode->i_ctime.tv_sec = inode->i_mtime.tv_sec = time(NULL);
    minix_sync_inode(inode);

    catch(e_zone_rw){ 
        kfree(blk);
        return nrbytes;
    }
}

void minix_write(object_t o,void *buffer,cnt_t count) {
    cnt_t nrbytes;
    struct file *file = self()->private_data;
    nrbytes = v2_write(file->inode,buffer,file->offset,count);
    if(nrbytes > 0)
        file->offset += nrbytes;
    ret(o,nrbytes); 
}

void minix_read(object_t o,void *buffer,cnt_t count){
    cnt_t nrbytes;
    struct file *file = self()->private_data;
    nrbytes = v2_read(file->inode,buffer,file->offset,count);
    if(nrbytes > 0)
        file->offset += nrbytes;
    ret(o,nrbytes); 
}
