#include "object.h"
#include "eval.h"
#include "util.h"
#include "cons.h"
#include "debug.h"

Object _if(Object sexp) {
    assert(sexp);
    Object condition = False;
    if(!typeis(sexp,Cons))
        error(,sexp,"can't be destructured against the lambda list.\n");
    condition = _car(sexp);
    if(!typeis(condition,Bool))
        error("The value ",condition," is not of the expected type Bool.\n");

    if(condition == True)
        return _eval(_car(_cdr(sexp)));
    else if(condition == False)
        return _eval(_car(_cdr(_cdr(sexp))));
    return Nil;
}
