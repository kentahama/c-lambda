%{
#include <stdio.h>
#include "lambda.h"
typedef struct lambda_s *lambda;
#define YYERROR_VERBOSE 1
int yylex();
void yyerror(const char *s);
%}


%union {
    void *m;
    char cval;
}
%token <cval>           VAR
%token                  LAM LF ARROW
%type   <m>             line expr term var lam app

%start                  line

%%
line : /*empty*/ { $$ = NULL; }
     | line expr LF { lambda m = $2; beta_reduction(&m); $$ = $1; };

expr : term | app;
term : var | lam | '(' expr ')' { $$ = $2; } ;
var : VAR { $$ = var($1); } ;
lam : LAM VAR ARROW expr { $$ = lam($2, $4); } ;
app : expr term { $$ = app($1, $2); } ;
%%

int main(void) {
    extern int yyparse(void);
    extern FILE *yyin;

    yyin = stdin;

    while (yyparse()) ;

    return 0;
}
