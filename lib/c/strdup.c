#include <string.h>
#include <stdlib.h>
extern char *strdup(const char *p)
{
    char *_new;
    if((_new = malloc(strlen(p) + 1)) != NULL)
        strcpy(_new,p);
    return _new;
}
