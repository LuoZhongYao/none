#ifndef __NOSH_CONS_H__
#define __NOSH_CONS_H__
extern Object _car(Object cons);
extern Object _cdr(Object cons);
#define dolist(v,lst) \
    for(Object v,l = lst;Nil != (v = _car(l));l = _cdr(l))
#endif
