#include "config.h"
#include <stdio.h>
#include "symbol.h"
#include "object.h"
#include "eval.h"
#include "cons.h"
#include "debug.h"
static Object do_eval(Object object,bool evl);

static Object symbol(Symbol symbol) {
    if(!symbol->binding)
        error(" ",NULL,"Unbound symbol: %s\n",symbol->key);
    return symbol->binding;
}

static Object lambda(Object lmb,Object vars) {
    assert(lmb);
    Object ret = Nil;
    dolist(var,lmb->impl.args) {
        Object value = _car(vars);
        if(!typeis(var,Symbol))
            error(,var," is not a symbol.\n");
        insert(var->impl.key,value);
        vars = _cdr(vars);
    }

    dolist(sexp,lmb->impl.fn) {
        sexp = copyObject(sexp);
        ret = _eval(sexp);
    }

    dolist(var,lmb->impl.args) {
        pop(var->impl.key);
    }
    return ret;
}

static Object apply(Object object) {
    Object car = object->impl.car;
    Object cdr = object->impl.cdr;
    Object fn = Nil;

    fn = _eval(car);
    if(typeis(car,Symbol)) {
        if(!eq(car->impl.key,"let") && !eq(car->impl.key,"lambda"))
            cdr = _eval(cdr);
    } else if(typeis(fn,Lambda)){
        cdr = _eval(cdr);
    } else {
        error("Car of ",car," is not a function name or lambda-expression.\n");
    }

    if(typeis(fn,Function))
        return fn->impl.function(cdr);

    if(typeis(fn,Lambda))
        return lambda(fn,cdr);

    return object;
}

static Object do_eval(Object object,bool evl) {
    assert(object);
    switch(object->type) {
    case Object_Nil:
    case Object_Number:
    case Object_String:
    case Object_Function:
    case Object_Lambda:
    case Object_Bool:
        break;
    case Object_Eval:
        object = do_eval(object->impl.eval,true);
        break;
    case Object_Quote:
        if(evl)
            object = do_eval(object->impl.quote,false);
        else
            object->impl.quote = do_eval(object->impl.quote,false);
        break;
    case Object_List:
        if(evl)
            object = apply(object);
        else {
            object->impl.car = do_eval(object->impl.car,evl);
            object->impl.cdr = do_eval(object->impl.cdr,evl);
        }
        break;
    case Object_Cons:
        object->impl.car = do_eval(object->impl.car,evl);
        object->impl.cdr = do_eval(object->impl.cdr,evl);
        break;
    case Object_Symbol:
        if(evl) {
            Symbol sym = lookup(object->impl.key);
            if(sym)
                object = symbol(sym);
            else
                error(" ",NULL,"Undefined symbol: %s\n",object->impl.key);
        }
        break;
    }
    return object;
}

Object _eval(Object object) { return do_eval(object,true); }

Object _echo(Object object) {
    if(object) {
        switch(object->type){
        case Object_Bool:
            printf("%s",object->impl._bool ? "TRUE" : "FALSE");
            break;
        case Object_Nil:
            printf("NIL");
            break;
        case Object_String:
            printf("%s",object->impl.string);
            break;
        case Object_Quote:
            printf("`");
            _echo(object->impl.quote);
            break;
        case Object_Number:
            printf("%ld",object->impl.number);
            break;
        case Object_List:
            printf("(");
            _echo(object->impl.car);
            if(object->impl.cdr != Nil) {
                printf(" ");
                _echo(object->impl.cdr);
            }
            printf(")");
            break;
        case Object_Cons:
            _echo(object->impl.car);
            if(object->impl.cdr != Nil) {
                printf(" ");
                _echo(object->impl.cdr);
            }
            break;
        case Object_Symbol:
            printf("%s",object->impl.key);
            break;
        case Object_Function:
            printf("<#function %p>",object->impl.function);
            break;
        case Object_Lambda:
            printf("(lambda ");
            _echo(object->impl.args);
            printf(" ");
            _echo(object->impl.fn);
            printf(")");
            break;
        case Object_Eval:
            printf(",");
            break;
        }
    }
    return Nil;
}
