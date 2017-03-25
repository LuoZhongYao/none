#include "object.h"
#include "symbol.h"
#include "cons.h"
#include "eval.h"
#include "debug.h"
Object _lambda(Object lmb){
    if(typeis(lmb,Cons)) {
        return newLambda(_cdr(lmb),_car(lmb));
    }
    error(,lmb," is not a lambda.\n");
    return Nil;
}
