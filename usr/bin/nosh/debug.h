#ifndef __NOSH_DEBUG_H__
#define __NOSH_DEBUG_H__

#ifdef DEBUG
#include <stdio.h>
#define dprint printf
#else
#define dprint(...)
#endif

#endif
