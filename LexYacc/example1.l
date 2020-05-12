%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "y.tab.h"
extern YYSTYPE yylval;
extern "C" {
	int yywrap(void);
	int yylex(void);
}
%}

eq [=]
add [+]
sub [-]
mul [*]
div [/]{1,2}
mod [%]
id [A-Za-z_][A-Za-z_0-9]*
intv [1-9][0-9]*
floatv [0-9]*\.[0-9]+
lrb [(]
rrb [)]
lab [<]
rab [>]
lsb [\[]
rsb [\]]
com [,]
sem [;]

%%
{eq} {return EQ;}
{add} {return ADD;}
{sub} {return SUB;}
{mul} {return MUL;}
{div} {return DIV;}
{mod} {return MOD;}
{id} {yylval.Sval = strdup(yytext); return ID;}
{intv} {yylval.Ival = atoi(yytext); return INTV;}
{floatv} {yylval.Fval = atof(yytext); return FLOATV;}
{lrb} {return LRB;}
{rrb} {return RRB;}
{lab} {return LAB;}
{rab} {return RAB;}
{lsb} {return LSB;}
{rsb} {return RSB;}
{com} {return COM;}
{sem} {return SEM;}
. {printf("Wrong Input!\n"); exit(1);}

%%
int yywrap(void) {
	return 1;
}
