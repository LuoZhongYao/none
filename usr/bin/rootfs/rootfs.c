#include <stdlib.h>
#include <stdio.h>

/*!  import by none !*/
#define  eprint(fmt,...)   printf(fmt"\n",##__VA_ARGS__)
#include <string.h>
#include "z.h"
#include "rootfs.h"
#include "sha1.h"

#define DIGIT_STRING    "0123456789abcdef"

static inline void *zalloc(size_t len){
    void *buffer = try(NULL == ,malloc(len),return NULL);
    memset(buffer,0,len);
    return buffer;
}

static struct hlist_head  dir[256] = {{(void*)-1}};

static void sha1ToString(char *str,const uint8_t *sha1){
    if(!str || !sha1) return;
    foreach(i,0,20){
        str[i * 2]  = DIGIT_STRING[(sha1[i] >> 4) & 0xf];
        str[i * 2 + 1]     = DIGIT_STRING[sha1[i] & 0xf];
    }
    str[40] = 0;
}

static void calcSHA1(void *sha1,void *data,size_t len){
    SHA1_CONTEXT hd;
    sha1_init(&hd);
    sha1_write(&hd,data,len);
    sha1_final(&hd);
    memcpy(sha1,hd.buf,20);
}

static void RFSHA1(RFS *rfs){
    calcSHA1(rfs->sha1,rfs->blob,rfs->size);
}

static void RFSName(char *name,RFS *rfs){
    sha1ToString(name,rfs->sha1);
}



RFS* newRFS(void *data,RFSType type ,size_t len){
    RFS *rfs;
    if(and(!,data,len)) return NULL;
    rfs = try(NULL == ,zalloc(sizeof(rfs[0]) + len),return NULL);
    rfs->size = len;
    rfs->type = type;
    INIT_HLIST_NODE(&(rfs->hlist));
    memcpy(rfs->blob,data,len);
    RFSHA1(rfs);
    hlist_add_head(&(rfs->hlist),&(dir[rfs->sha1[0]]));
    return rfs;
}

#define CALCRFS(x)  (sizeof(x[0]) + x->size)

RFS *newRFSTree(RFS *old,const RFS *new,const char *name){
    RFSTree *tree;
    if(or(!,name,new)) return NULL;

    size_t size = 0;
    if(old){
        if(old->type != RFS_TREE) return NULL;
        size = old->size;
        tree = zalloc(size + sizeof(RFSTree));
        memcpy(tree,old->blob,size);
    }else{
        tree = zalloc(sizeof(RFSTree));
    }
    int index = size / sizeof(RFSTree);
    size_t nLen = strlen(name);

    memcpy(tree[index].sha1,new->sha1,20);
    memcpy(tree[index].name,name,nLen > RFS_NAME_LENGTH ? RFS_NAME_LENGTH : nLen);
           tree[index].type = new->type;
    return newRFS(tree,RFS_TREE,size + sizeof(RFSTree));
}

int main(int argc,char **argv){
#if 0
    char *argv[] = {
        "rootfs","/gpl.txt", "/none.txt",
    };
    int argc = ARRAY_SIZE(argv);
#endif
    int fd = -1;
    off_t len = 1024;
    void *buffer = NULL;
    if(argc < 2) 
        return -1;
    RFS *rfs = NULL;
    foreach(i,0,ARRAY_SIZE(dir)){
        dir[i].first = NULL;
    }
    foreach(i,1,argc){
        len = 1024;
        RFS *new;
        fd = try(0 > ,open(argv[i],0),{
            throw e_fial;
        });
        buffer = try(NULL == ,zalloc(len),{
            throw e_fial;
        });
        len = read(fd,buffer,len);
        new = newRFS(buffer,RFS_BLOB,len);
        rfs = newRFSTree(rfs,new,argv[i]);
        catch(e_fial){
            TEST_AND_FREE(free,buffer,NULL);
            TEST_AND_FREE(close,fd,-1);
            continue;
        }
    }

    foreach(i,0,ARRAY_SIZE(dir)){
        char sha1[41];
        struct hlist_node *r;
        hlist_for_each(r,&dir[i]){
            rfs = hlist_entry(r,RFS,hlist);
            RFSName(sha1,rfs);
            printf("%s %s %2x\n",((char *[]){"BLOB","TREE"})[rfs->type],sha1,i);
            if(rfs->type == RFS_TREE){
                for(__typeof__(sizeof(RFSTree))j = 0;rfs && j < rfs->size / sizeof(RFSTree);j++){
                    char sha1[41];
                    sha1ToString(sha1,rfs->tree[j].sha1);
                    printf("  %s %s %s ","|-",((char *[]){"BLOB","TREE"})[rfs->tree[j].type],sha1);
                    foreach(k,0,RFS_NAME_LENGTH)
                        printf("%c",rfs->tree[j].name[k]);
                    printf("\n");
                }
            }else{
                foreach(j,0,rfs->size){
                    printf("%c",rfs->blob[j]);
                }
            }
        }
    }
    return 0;
}
