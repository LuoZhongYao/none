#ifndef __NOSH_EVAL_H__
#define __NOSH_EVAL_H__
#include "object.h"
Object _eval(Object object);
Object _echo(Object object);
#define error(prefix,o,fmt,...)  ({ dprint("Error: "prefix);_echo(o);dprint(fmt,##__VA_ARGS__);exit(1); })
#endif
