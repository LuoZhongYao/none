#ifndef __ROOTFS_H__
#define __ROOTFS_H__
#include <stdint.h>
#include <none/list.h>
#define     RFS_NAME_LENGTH (256 - 20 - sizeof(void*))

typedef enum{RFS_BLOB,RFS_TREE}RFSType;
typedef struct{
    RFSType type;
    uint8_t sha1[20];
    uint8_t name[RFS_NAME_LENGTH];
}RFSTree;

typedef struct{
    RFSType type;
    size_t  size;
    uint8_t sha1[20];
    struct hlist_node hlist;
    union{
        RFSTree tree[0];
        char    blob[0];
    };
}RFS;

#endif
