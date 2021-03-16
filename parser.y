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
	struct expression *expr;
	char *identifier;
	int token;
}

%token <identifier> IDENTIFIER
%token <token> TYPE RETURN NUMBER ERROR

%type <func> function
%type <bdy> body
%type <stmt> statement
%type <expr> expression
%type <token> unop

%%

program: /* nothing */
       | program function {
	fprintf(outfile, "\t.text\n");
	writefunc($2, outfile);
	clearfunc($2);
}
;

function: TYPE IDENTIFIER '(' ')' '{' body '}' {
	$$ = creatfunc($6, TYPE, $2); }
;

body: statement { $$ = creatbdy($1); }
    | body statement { $$ = addstmt($1, $2); }
;

statement: RETURN expression ';' { $$ = creatstmt($2, 0); }
;

expression: NUMBER { $$ = createxpr(creatop(0, $1)); }
    | unop expression { $$ = addop($2, creatop($1, 0)); }
;

unop: '-' { $$ = '-'; }
    | '!' { $$ = '!'; }
    | '~' { $$ = '~'; }
;

%%

int yyerror(char *s)
{
	fprintf(stderr, "error: %s\n", s);
	return 0;
}
