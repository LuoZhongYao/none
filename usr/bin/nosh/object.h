#ifndef __NOSH_OBJECT_H__
#define __NOSH_OBJECT_H__
#include "types.h"
#include "util.h"

extern int yylineno;
void yyerror(const char *s,...);

typedef struct _Object* Object;

typedef enum {
    Object_Nil,
    Object_Cons,
    Object_List,
    Object_Eval,
    Object_Bool,
    Object_Quote,
    Object_Number,
    Object_String,
    Object_Symbol,
    Object_Lambda,
    Object_Function,
} ObjectType;

#define tn(t)    [Object_##t] = #t
#define typename(o) ((char *[]){\
        tn(Nil),tn(Cons),tn(List),tn(Quote),tn(Number),tn(String),tn(Symbol),\
        tn(Function),tn(Lambda),tn(Bool),\
        })[o->type]

typedef union {
    long   number;
    bool   _bool;
    String key;
    String string;
    Object eval;
    Object quote;
    struct { Object car,cdr; };
    struct { Object fn,args; };
    Object (*function)(Object);
}ObjectImpl;

struct _Object {
    ObjectType type;
    ObjectImpl impl;
};

#define _typeis(o,t)    (o->type == Object_##t)
#define typeis(o,t)     _typeis(o,t)

extern Object Nil;
extern Object True;
extern Object False;

extern Object newObject(ObjectType type,ObjectImpl impl);
extern Object newList(Object lst);
extern Object newCons(Object car,Object cdr);
extern Object newEval(Object evl);
extern Object newQuote(Object quote);
extern Object newNumber(long number);
extern Object newString(String string); 
extern Object newSymbol(String key); 
extern Object newLambda(Object fn,Object args);
extern Object newFunction(Object (*fn)(Object));
extern Object copyObject(Object object);
void initialObject(void);

#endif
