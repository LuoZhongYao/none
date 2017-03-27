#include <stddef.h>
#define DEFINE(sym, val) \
	__asm__ __volatile__("\n->" #sym " %0 " #val : : "i" (val))

#define BLANK() __asm__ __volatile__("\n->" : : )

#define OFFSET(sym, str, mem) \
	DEFINE(sym, offsetof(struct str, mem));

