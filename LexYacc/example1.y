%{
#include <stdio.h>
#include <string.h>
#include "lex.yy.c"
#include "node.h"
extern "C" {
	void yyerror(const char* s);
	extern int yylex(void);
}
%}

%union {
	int Ival;
	float Fval;
	char* Sval;
	struct TreeNode* TNval;
}
%start A
%token <Ival> INTV
%token <Fval> FLOATV
%token <Sval> EQ ADD SUB MUL DIV MOD ID LRB RRB LAB RAB LSB RSB COM SEM
%type <TNval> A P S LHS RHS Term Factor TRef SRef CList AList IdExpr T F Const

%%
A: P {TreeRoot = $1;}
P: S {$$ = $1;}
| P S {$$ = new_node(Com, $1, $2);}
;
S: LHS EQ RHS SEM {$$ = new_node(Eq, $1, $3);}
;
LHS: TRef {$$ = $1;}
;
RHS: Term {$$ = $1;}
| RHS ADD Term {$$ = new_node(Add, $1, $3);}
| RHS SUB Term {$$ = new_node(Sub, $1, $3);}
;
Term: Factor {$$ = $1;}
| Term MUL Factor {$$ = new_node(Mul, $1, $3);}
| Term DIV Factor {$$ = new_node(Div, $1, $3);}
| Term MOD Factor {$$ = new_node(Mod, $1, $3);}
;
Factor: Const {$$ = $1;}
| SRef {$$ = $1;}
| TRef {$$ = $1;}
| LRB RHS RRB {$$ = $2;}
;
TRef: ID LAB CList RAB LSB AList RSB {TreeNodeVal val; val.String = strdup($1); $$ = new_valnode(Tref, $3, $6, val); free($1); free(val.String);}
;
SRef: ID LAB CList RAB {TreeNodeVal val; val.String = strdup($1); $$ = new_valnode(Sref, $3, NULL, val); free($1); free(val.String);}
;
CList: INTV {TreeNodeVal val; val.Int = $1; $$ = new_valnode(Intv, NULL, NULL, val);}
| CList COM INTV {TreeNodeVal val; val.Int = $3; $$ = new_node(Com, $1, new_valnode(Intv, NULL, NULL, val));}
;
AList: IdExpr {$$ = $1;}
| AList COM IdExpr {$$ = new_node(Com, $1, $3);}
;
IdExpr: T {$$ = $1;}
| IdExpr ADD T {$$ = new_node(Add, $1, $3);}
;
T: F {$$ = $1;}
| T MUL F {$$ = new_node(Mul, $1, $3);}
| T DIV F {$$ = new_node(Div, $1, $3);}
| T MOD F {$$ = new_node(Mod, $1, $3);}
;
F: ID {TreeNodeVal val; val.String = strdup($1); $$ = new_valnode(Index, NULL, NULL, val); free($1); free(val.String);}
| INTV {TreeNodeVal val; val.Int = $1; $$ = new_valnode(Intv, NULL, NULL, val);}
| LRB IdExpr RRB {$$ = $2;}
;
Const: INTV {TreeNodeVal val; val.Int = $1; $$ = new_valnode(Intv, NULL, NULL, val);}
| FLOATV {TreeNodeVal val; val.Float = $1; $$ = new_valnode(Floatv, NULL, NULL, val);}
;

%%
void yyerror(const char* s) {
	fprintf(stderr, "%s", s);
}
