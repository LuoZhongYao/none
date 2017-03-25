#include "object.h"
#include "eval.h"
Object _car(Object cons) {
    if(typeis(cons,Cons) || typeis(cons,List))
        return cons->impl.car;
    return Nil;
}

Object _cdr(Object cons) {
    if(typeis(cons,Cons))
        return copyObject(cons->impl.cdr);
    if(typeis(cons,List)) {
        Object n = copyObject(cons->impl.cdr);
        if(n != Nil)
            return newList(n);
    }
    return Nil;
}
