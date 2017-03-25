#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "symbol.h"
#include "util.h"
#include "config.h"

#define error(fmt,...) ({ printf(fmt,##__VA_ARGS__); exit(1);})
static Symbol table[NHASH] = {NULL};

static unsigned int hash(String s) {
    unsigned hash = 0;
    unsigned c;
    while(0 != (c = *s++)) 
        hash = hash * 9 ^ c;
    return hash;
}

static Symbol symbol(String key,Object binding,Symbol next) {
    Symbol symbol = cmalloc(sizeof(Symbol));
    symbol->key = key;
    symbol->binding = binding;
    symbol->next = next;
    return symbol;
}

Symbol insert(String key,Object binding) {
    int index = hash(key) % NHASH;
    table[index] = symbol(key,binding,table[index]);
    return table[index];
}

void pop(String key) {
    assert(symbol);
    int index = hash(key) % NHASH;
    Symbol *head = &table[index];
    for(;*head;head = &((*head)->next)) {
        if(eq((*head)->key,key)) {
            *head = (*head)->next;
            break;
        }
    }
}

Symbol lookup(String key) {
    int index = hash(key) % NHASH;
    Symbol symbol;
    for(symbol = table[index];symbol;symbol = symbol->next) {
        if(eq(symbol->key,key))
            return symbol;
    }
    return NULL;
}
