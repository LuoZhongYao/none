%{
#include <stdio.h>
#include "object.h"
#include "eval.h"
#include "util.h"
#define YYSIZE_T size_t
#define YYCOPY memcpy

extern int yylex(void);

%}

%union {
    long    number;
    String  string;
    Object  object;
}

%token <number>NUMBER
%token <string>SYMBOL
%token <string>STRING
%type  <object>atom sexp list sexp_list

%start start

%%

start
    : sexp              {_echo(_eval($1));printf("\n? ");}
    | start sexp        {_echo(_eval($2));printf("\n? ");}
    ;

sexp
    : atom
    | list
    | ',' sexp          { $$ = newEval($2);  }
    | '`' sexp          { $$ = newQuote($2); }
    ;

list
    : '(' ')'           { $$ = newList(Nil);}
    | '(' sexp_list ')' { $$ = newList($2);  }
    ;

atom
    : NUMBER            { $$ = newNumber($1); }
    | SYMBOL            { $$ = newSymbol($1); }
    | STRING            { $$ = newString($1); }
    ;

sexp_list
    : sexp
    | sexp sexp_list    { $$ = newCons($1,$2); }
    ;
    
%%
void yyerror(const char *str,...) {
    printf("lisp : %s\n",str);
}

