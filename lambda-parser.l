%{
#include "lambda-parser.tab.h"
%}

whitespace      [ \t]
letter          [a-zA-Z]
paren           [()]
other           .

%%
{whitespace}+   { /* ignore */ };
"\\"            { return LAM; };
{letter}+       { yylval.cval = yytext[0]; return VAR; };
"->"            { return ARROW; };
{paren}         { return yytext[0]; };
"\n"            { return  LF; };
{other}         { fprintf(stderr, "Illegal character %c, ignored\n", yytext[0]); };
%%
