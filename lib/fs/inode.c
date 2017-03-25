#include <fs/fs.h>

void drop_nlink(struct inode *inode) {
    if(inode->i_nlinks)
        inode->i_nlinks--;
}

void clear_nlink(struct inode *inode) {
    if(inode->i_nlinks)
        inode->i_nlinks = 0;
}

void set_nlink(struct inode *inode,unsigned long nlink){
    if(!nlink)
        clear_nlink(inode);
    else
        inode->i_nlinks = nlink;

}

void inc_nlink(struct inode *inode) {
    inode->i_nlinks++;
}
