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
%right EQ
%left ADD SUB
%left MUL FDIV IDIV MOD
%token LRB RRB LAB RAB LSB RSB COM SEM
%token <Ival> INTV
%token <Fval> FLOATV
%token <Sval> ID
%type <TNval> P S LHS RHS TRef SRef CList AList IdExpr Const

%%
A: P {TreeRoot = $1;}
P: S {$$ = $1;}
| P S {$$ = new_node(Com, $1, $2);}
;
S: LHS EQ RHS SEM {$$ = new_node(Eq, $1, $3);}
;
LHS: TRef {$$ = $1;}
;
RHS: Const {$$ = $1;}
| SRef {$$ = $1;}
| TRef {$$ = $1;}
| RHS ADD RHS {$$ = new_node(Add, $1, $3);}
| RHS SUB RHS {$$ = new_node(Sub, $1, $3);}
| RHS MUL RHS {$$ = new_node(Mul, $1, $3);}
| RHS FDIV RHS {$$ = new_node(Div, $1, $3);}
| RHS IDIV RHS {$$ = new_node(Div, $1, $3);}
| RHS MOD RHS {$$ = new_node(Mod, $1, $3);}
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
IdExpr: ID {TreeNodeVal val; val.String = strdup($1); $$ = new_valnode(Index, NULL, NULL, val); free($1); free(val.String);}
| IdExpr ADD IdExpr {$$ = new_node(Add, $1, $3);}
| IdExpr SUB IdExpr {$$ = new_node(Sub, $1, $3);}
| IdExpr ADD INTV {TreeNodeVal val; val.Int = $3; $$ = new_node(Add, $1, new_valnode(Intv, NULL, NULL, val));}
| IdExpr MUL INTV {TreeNodeVal val; val.Int = $3; $$ = new_node(Mul, $1, new_valnode(Intv, NULL, NULL, val));}
| IdExpr IDIV INTV {TreeNodeVal val; val.Int = $3; $$ = new_node(Div, $1, new_valnode(Intv, NULL, NULL, val));}
| IdExpr MOD INTV {TreeNodeVal val; val.Int = $3; $$ = new_node(Mod, $1, new_valnode(Intv, NULL, NULL, val));}
| LRB IdExpr RRB {$$ = $2;}
;
Const: INTV {TreeNodeVal val; val.Int = $1; $$ = new_valnode(Intv, NULL, NULL, val);}
| FLOATV {TreeNodeVal val; val.Float = $1; $$ = new_valnode(Floatv, NULL, NULL, val);}
;

%%
void yyerror(const char* s) {
	fprintf(stderr, "%s", s);
}
