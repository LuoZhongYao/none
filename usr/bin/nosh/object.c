#include "object.h"
#include "eval.h"
#include "cons.h"
#include "symbol.h"
#include "debug.h"

#define objImpl(...)    (ObjectImpl){__VA_ARGS__}
Object Nil;
Object True;
Object False;

#define copy(o) newObject(o->type,o->impl)

Object copyObject(Object object) {
    assert(object);

    if(typeis(object,Nil))
        return Nil;

    object = copy(object);

    if(typeis(object,List) || typeis(object,Cons)) {
        object->impl.car = copyObject(object->impl.car);
        object->impl.cdr = copyObject(object->impl.cdr);
    }
    return object;
}

Object newObject(ObjectType type,ObjectImpl impl) {
    Object object = cmalloc(sizeof(*object));
    object->type = type;
    object->impl = impl;
    return object;
}

Object newList(Object lst) {
    if(lst != Nil) {
        if(!typeis(lst,Cons))
            lst = newCons(lst,Nil);
        else {
            Object *cdr = &lst;
            while(typeis((*cdr)->impl.cdr,Cons))
                cdr = &((*cdr)->impl.cdr);
            (*cdr)->impl.cdr = newCons((*cdr)->impl.cdr,Nil);
        }
        lst->type = Object_List;
    }
    return lst;
}

Object newCons(Object car,Object cdr) {
    return newObject(Object_Cons  ,objImpl(.car = car,.cdr = cdr));
}

extern Object newEval(Object eval)              { return newObject(Object_Eval  ,objImpl(.eval = eval));       }
extern Object newQuote(Object quote)            { return newObject(Object_Quote ,objImpl(.quote = quote));     }
extern Object newNumber(long number)            { return newObject(Object_Number,objImpl(.number = number));   }
extern Object newString(String string)          { return newObject(Object_String,objImpl(.string = string));   }
extern Object newSymbol(String key)             { return newObject(Object_Symbol,objImpl(.string = key));    }
extern Object newLambda(Object fn,Object args)  { return newObject(Object_Lambda,objImpl(.fn = fn,.args = args));}
extern Object newFunction(Object (*fn)(Object)) { return newObject(Object_Function,objImpl(.function = fn));    }
static Object newNil(void)                      { return newObject(Object_Nil,objImpl(.number = 0));           }
static Object newBool(bool _bool)               { return newObject(Object_Bool,objImpl(._bool = _bool));}

static Object _atom(Object sexp) {
    assert(sexp);
    sexp = _car(sexp);
    switch(sexp->type){
    case Object_String:
    case Object_Number:
    case Object_Nil:
    case Object_Bool:
        return True;
        break;
    default:
        return False;
    }
    return False;
}

static Object _eq(Object sexp) {
    assert(sexp);
    if(typeis(sexp,Cons)) {
        Object a = _car(sexp);
        Object b = _car(_cdr(sexp));

        if(a == b)
            return True;

        if(a->type == b->type)
            switch(a->type){
            case Object_Number:
                if(a->impl.number == b->impl.number)
                    return True;
                break;
            default:
                break;
            }
    }
    return False;
}

static Object _quit(Object sexp) {
    exit(0);
    return Nil;
    (void)sexp;
}

static Object __car(Object sexp) {
    return _car(_car(sexp));
}

static Object __cdr(Object sexp) {
    return _cdr(_car(sexp));
}

extern Object _let(Object);
extern Object _lambda(Object);
extern Object _define(Object);
extern Object _if(Object);
void initialObject(void) {
    Nil     = newNil();
    False   = newBool(false);
    True    = newBool(true);
    insert("nil",   Nil);
    insert("false" ,False);
    insert("true",  True);
    insert("car",   newFunction(__car));
    insert("cdr",   newFunction(__cdr));
    insert("echo",  newFunction(_echo));
    insert("atom",  newFunction(_atom));
    insert("eq",    newFunction(_eq));
    insert("quit",  newFunction(_quit));
    insert("let",   newFunction(_let));
    insert("eval",  newFunction(_eval));
    insert("lambda",newFunction(_lambda));
    insert("define",newFunction(_define));
    insert("if",    newFunction(_if));
}
