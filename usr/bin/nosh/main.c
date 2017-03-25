#include "object.h"
#include <stdio.h>

int yyparse (void);
int main(void) {
    initialObject();
    printf("? ");
    yyparse();
}
