#include    <stdio.h>
#include    <stdlib.h>
#include    <stdint.h>
#include    <string.h>
#define eprint(fmt,...)  printf(fmt"\n",##__VA_ARGS__)
#include    <z.h>

#define mm_error(fmt,...)   printf("[MALLOC] : "fmt"\n",##__VA_ARGS__)
#define mm_warning(fmt,...) printf("[MALLOC] : "fmt"\n",##__VA_ARGS__)


#define M_BUSY      1
#define M_UNBUSY    0

#ifndef typeof
#define typeof  __typeof__
#endif

#define log2(n) ({                      \
        int _v;                         \
        for(_v = 0;n > (1 << _v);_v++); \
        _v; })
#define exp2(n) (1 << n)

#define NR_MOBJECT              32
#define HEAP_LIMIT              ((MObject*)(((void*)heap->chunk) + heap->length))
#define AFTER(x)                ((MObject*)(((void*)(x)) + (x)->length))
#define isHeapOverflow(x)       (AFTER(x) >= HEAP_LIMIT)
#define isLog2Overflow(x)       ((x) >= NR_MOBJECT || (x) < 0)

#define _TAIL(x)    ({          \
        if(!isHeapOverflow(x)){             \
            AFTER(x)->before = x;   \
        }})

typedef struct _MObject MObject;
typedef struct _MHeap   MHeap;

struct _MObject{
    MObject     *before;    /*!指向上一个对象,由上一个对象维护(什么,这个对象的成员由上个对象维护?)!*/
    long        busy;
    size_t      length;     /*! 整个对象长度,包括prev,length,所占空间在内 !*/
    union{
        struct{MObject *next,*prev;};   /*! 空闲对象链 !*/
        int8_t  chunk[0];               /*! 对象管理的数据块 !*/
    };
};

struct _MHeap{
    MObject *objectList[NR_MOBJECT];
    size_t  length;
    int8_t  chunk[0];
}*heap = NULL;

/*******************************************************************************
 *  delMObject
 *      从链表里面删除节点,并返回被删除的节点
 *  ARGS:
 *      mobject     将被删除的节点
 * *****************************************************************************/
static inline MObject *delMObject(MObject *mobject){
    MObject *next,*prev;
    if(!(mobject)){
        mm_error("Trying to remove an empty object");
        return NULL;
    }
    next = mobject->next;
    prev = mobject->prev;
    if(!!(prev)){
        prev->next = next;
    }else{
        heap->objectList[log2(mobject->length)] = next;
    }
    if(!!(next)){
        next->prev = prev;
    }
    mobject->busy = M_BUSY;
    return mobject;
}

static inline MObject *insertMObject(MObject *head,MObject *new){
    if(!(new)){
        mm_error("Trying to insert an empty object");
        return NULL;
    }
    new->next = head;
    new->prev = NULL;
    new->busy = M_UNBUSY;
    if(!!(head)){
        head->prev = new;
    }
    return new;
}

static inline MObject *splitMObject(MObject *mobject){
    int log2n;
    size_t nl;
    MObject *tail;
    if(!(mobject)){
        mm_error("Trying to split an empty object");
        return NULL;
    }
    nl = mobject->length >> 1;
    log2n = log2(nl);
    tail = ((void*)mobject) + nl;
    *tail = (MObject){
        .before = mobject,
        .length = nl,
    };
    _TAIL(tail);
    heap->objectList[log2n] = insertMObject(heap->objectList[log2n],tail);
    mobject->length = nl;
    return mobject;
}

static inline MObject *mergerMObject(MObject *head,MObject *tail){
    if(!(head) || !(tail)){
        mm_error("Trying to merger emptry objects");
        exit(1);
    }
    if(AFTER(head) != tail){
        mm_error("Trying to merger noncontiguous objects\nhead : %p length : %x\ntail : %p length : %x",head,head->length,tail,tail->length);
        exit(1);
    }
    if(head->length == tail->length){
        head->length <<= 1;
        _TAIL(head);
    }
    return head;
}

static MObject *getMObject(int log2n){
    MObject *mobject;
    if(isLog2Overflow(log2n)){
        return NULL;
    }
    mobject = heap->objectList[log2n];
    if(mobject){
        mobject = delMObject(mobject);
        return mobject;
    }
    mobject = getMObject(log2n + 1);
    if(!(mobject)){
        return NULL;
    }
    return splitMObject(mobject);
}

static int realizeMHeap(size_t length){
    int log2n;
    MObject *mobject;
    if(heap){
        mm_error("Heap already exists,address is %p and &heap is %p",heap,&heap);
        return -1;
    }
    length += (sizeof(MHeap));
    log2n = log2(length);
    if(isLog2Overflow(log2n)){
        mm_error("Can only create heap 0 ~ %d,but requesting %d",exp2(31) - sizeof(MHeap),length);
        return -1;
    }
    extern char _end;
    heap = (void*)&_end; /*! malloc(sizeof(MHeap) + exp2(log2n)); !*/
    if(!(heap)){
        mm_error("Oops,the memory is full,tell your boss");
        return -1;
    }
    memset(heap,0,sizeof(MHeap) + exp2(log2n));
    heap->length = exp2(log2n);
    mobject = (MObject *)heap->chunk;
    *mobject = (MObject){
        .before = NULL,
        .length = exp2(log2n),
        .busy   = M_UNBUSY,
        .next   = NULL,
        .prev   = NULL,
    };
    heap->objectList[log2n] = mobject;
    return 0;
}

static void *domalloc(size_t length){
    int log2n;
    MObject *mobject;
    length += (sizeof(MObject) - sizeof(struct{MObject *next,*prev;}));
    log2n = log2(length);
    if(isLog2Overflow(log2n)){
        mm_error("Can only alloc  0 ~ %d,but requesting %d",exp2(31) - sizeof(MHeap) - sizeof(MObject),length);
        return NULL;
    }
    mobject = getMObject(log2n);
    if(mobject){
        return mobject->chunk;
    }
    return NULL;
}

void free(void *ptr){
    int log2n;
    MObject *mobject = container_of(ptr,MObject,chunk);
    if(!(ptr) || mobject > (MObject*)HEAP_LIMIT || mobject < (MObject*)heap->chunk){
        mm_error("Trying to free non-heap memory %p",ptr);
        return ;
    }
    if((mobject->before)){
        MObject *head = mobject->before;
        if(head->busy == M_UNBUSY){
            head = delMObject(head);
            mobject = mergerMObject(head,mobject);
        }
    }
    if(!isHeapOverflow(mobject)){
        MObject *tail = AFTER(mobject);
        if(tail->busy == M_UNBUSY){
            tail = delMObject(tail);
            mobject = mergerMObject(mobject,tail);
        }
    }
    log2n = log2(mobject->length);
    heap->objectList[log2n] = insertMObject(heap->objectList[log2n],mobject);
}

static void *realize(size_t length);
static void *(*alloc)(size_t) = realize; 
static void *realize(size_t length){
    try(0 > ,realizeMHeap(0x400000),{
        return NULL;
    });
    alloc = domalloc;
    return domalloc(length);
};
void* malloc(size_t length){
    return alloc(length);
};
