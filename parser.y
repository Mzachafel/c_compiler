%{

#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

extern int yylex();
extern int yyerror(char *s);
extern FILE *outfile;

%}

%union {
	struct function *func;
	struct body *bdy;
	struct statement *stmt;
	char *id;
	int token;
}

%token <id> ID
%token <token> LBRACE RBRACE
%token <token> LPAREN RPAREN
%token <token> SEMICOL INT RET
%token <token> INTLIT UNDEFINED

%type <func> function
%type <bdy> body
%type <stmt> statement
%type <token> expr

%%

program: /* nothing */
       | program function {
	fprintf(outfile, "\t.text\n");
	writefunc($2, outfile);
	clearfunc($2);
}
;

function: INT ID LPAREN RPAREN LBRACE body RBRACE {
	$$ = creatfunc($6, INT, $2); }
;

body: statement { $$ = creatbdy($1); }
    | body statement { $$ = addstmt($1, $2); }
;

statement: RET expr SEMICOL { $$ = creatstmt(RET, $2); }
;

expr: INTLIT
;

%%

int yyerror(char *s)
{
	fprintf(stderr, "error: %s\n", s);
	return 0;
}
