#ifndef __NOSH_UTIL_H__
#define __NOSH_UTIL_H__
#include <stdlib.h>
//#include <assert.h>
#define assert(exp)
#include "ntl.h"
#include <string.h>
static inline void *cmalloc(int size) {
    void * ptr = malloc(size);
    assert(ptr);
    memset(ptr,0,size);
    return ptr;
}
#define eq(a,b)        (0 == strcmp(a,b))
#endif
