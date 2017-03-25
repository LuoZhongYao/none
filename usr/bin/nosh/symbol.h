#ifndef __NOSH_SYMBOL_H__
#define __NOSH_SYMBOL_H__
#include "types.h"

typedef struct _Object* Object;
typedef struct _Symbol* Symbol;

struct _Symbol {
    String key;
    Object binding;
    Symbol next;
};

Symbol insert(String key,Object binding);
Symbol lookup(String s);
void pop(String key);
#endif
