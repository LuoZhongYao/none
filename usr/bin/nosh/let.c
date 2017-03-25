#include "object.h"
#include "symbol.h"
#include "cons.h"
#include "eval.h"
#include "debug.h"

Object _let(Object args) {
    assert(args);
    Object symbols = Nil;
    Object body = Nil;
    Object ret = Nil;

    if(typeis(args,Cons)) {
        symbols = _car(args);
        body = _cdr(args);
    } else {
        symbols = args;
        body = Nil;
    }

    dolist(arg,symbols) {
        Object var = _car(arg);
        Object value = _eval(_car(_cdr(arg)));
        if(!typeis(var,Symbol))
            error(,var," is not a symbol.\n");
        insert(var->impl.key,value);
    }

    dolist(sexp,body){
        ret = _eval(sexp);
    }

    dolist(arg,symbols) {
        Object var = _car(arg);
        pop(var->impl.key);
    }

    return ret;
}
