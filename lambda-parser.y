%{
#include <stdio.h>
#include "lambda.h"
typedef struct lambda_s *lambda;
%}


%union {
    void *m;
    char cval;
}
%token <cval>		VAR
%token			LAM LF ARROW
%type	<m>		line expr var lam app

%start			line

%%
line : LF { $$ = NULL; } | expr line { lambda m = $1; beta_reduction(&m); $$ = $2; };

expr : var | lam  | app | '(' expr ')' { $$ = $2; } ;
var : VAR { $$ = var($1); } ;
lam : LAM VAR ARROW expr { $$ = lam($2, $4); } ;
app : expr expr { $$ = app($1, $2); } ;
%%

int main(void) {
    extern int yyparse(void);
    extern FILE *yyin;

    yyin = stdin;

    if (yyparse()) {
	fprintf(stderr, "An error occured, aborting.\n");
	return -1;
    }
    return 0;
}
