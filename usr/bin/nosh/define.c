#include "object.h"
#include "cons.h"
#include "eval.h"
#include "symbol.h"
#include "util.h"
#include "debug.h"
Object _define(Object sexp) {
    assert(sexp);
    Object symbol = Nil;
    Object value = Nil;
    if(!typeis(sexp,Cons))
        error(,sexp," can't be destructured against the lambda list.\n");
    symbol = _car(sexp);
    if(!typeis(symbol,Symbol))
        error("The value ",symbol," is not of the expected type Symbol.\n");
    value = _car(_cdr(sexp));
    insert(symbol->impl.key,value);
    return value;
}
