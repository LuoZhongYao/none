#ifndef __STDLIB_H__
#define __STDLIB_H__
#include <none/types.h>
#include <stddef.h>

/*
//extern void exit(int);
extern void exit() __attribute__((noreturn));
extern pid_t fork(void);
*/

extern pid_t fork(void);
extern int   open(String,int,...);
extern int   read(int,void*,int);
extern ssize_t   write(int,const void*,size_t);
extern int   close(object_t);
extern void *malloc(size_t size);
extern void free(void *ptr);
extern void *calloc(size_t nmemb, size_t size);
extern void *realloc(void *ptr, size_t size);
#define abort() exit(0)
extern int abs(int j);
extern int   atoi(const char *);
extern void  srand(unsigned seed);
extern int   rand(void);
extern off_t lseek(object_t ,off_t ,int);
extern int   exec(const char *path,int argc,char **argv);
extern int   execvp(const char *path,char **argv);
extern _Noreturn void exit(int status);
extern long strtol(const char *nptr, char **endptr, int base);
extern int atoi(const char *str);
#endif
