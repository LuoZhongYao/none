%option nodefault yylineno
%top {
#include <string.h>
#include "object.h"
#include "util.h"
static object_t STDIN = 0,STDOUT = 1;
FILE *stdin = &STDIN,*stdout = &STDOUT;
}


%{
#include "object.h"
#include "parser.h"
%}

d       [0-9]
l       [a-zA-Z_]
h       [a-fA-F0-9]
s       [ \t\n\v\f]
a       [()`,=@&:]

%%

";"[^\n]*   { /* comment */ ; }
[ \t\n\v\f] { ; }

{a}         { return yytext[0]; }

0[xX]{h}+   { yylval.number = atoi(yytext); return NUMBER; }
0[0-7]+     { yylval.number = atoi(yytext); return NUMBER; }
[1-9]{d}*   { yylval.number = atoi(yytext); return NUMBER; }

\"(\\.|[^\\"\n])*\"   { yytext[yyleng - 1] = 0;yylval.string = strdup(yytext + 1); return STRING; }
[^ \t\v\n\f()`,=@&:]+ { yylval.string = strdup(yytext); return SYMBOL; }

%%

int yywrap(void) {
    (void)input;
    (void)yyunput;
    return 1;
}
