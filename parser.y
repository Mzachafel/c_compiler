%{

#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

extern int yylex();
extern int yyerror(char *s);
extern FILE *outfile;
union trmval {
	struct expression *expr;
	struct term *trm;
	int num;
} val;

%}

%union {
	struct function *func;
	struct body *bdy;
	struct statement *stmt;
	struct expression *expr;
	struct factor *fctr;
	struct term *trm;
	char *type;
	char *identifier;
	int number;
	char unop;
}

%token <identifier> IDENTIFIER
%token <type> TYPE
%token <number> NUMBER 
%token RETURN ERROR

%type <func> function
%type <bdy> body
%type <stmt> statement
%type <expr> expression
%type <fctr> factor
%type <trm> term
%type <unop> unary_op

%%

program: /* nothing */
       | program function {
           fprintf(outfile, "\t.text\n");
	   writefunc($2, outfile);
	   clearfunc($2);
       }
;

function:
	TYPE IDENTIFIER '(' ')' '{' body '}' {
	    $$ = creatfunc($1, $2, $6); 
        }
;

body: 
    statement {
        $$ = creatbdy($1);
    }
    | body statement {
        $$ = addstmt($1, $2);
    }
;

statement:
	 RETURN expression ';' { 
	     $$ = creatstmt($2, 0);
         }
;

expression: 
	  factor {
              $$  = createxpr(0, 0, $1);
          }
	  | expression '+' factor {
	      $$ = createxpr($1, '+', $3);
	  }
	  | expression '-' factor {
	      $$ = createxpr($1, '-', $3);
	  }
;

factor:
      term {
          $$ = creatfctr(0, 0, $1);
      }
      | factor '*' term {
          $$ = creatfctr($1, '*', $3);
      }
      | factor '/' term {
          $$ = creatfctr($1, '/', $3);
      }
;

term: 
    NUMBER {
	val.num = $1;
        $$ = creattrm(val, 0);
    }
    | unary_op term {
	val.trm = $2;
        $$ = creattrm(val, $1);
    }
    | '(' expression ')' {
	val.expr = $2;
        $$ = creattrm(val, 1);
    }
;

unary_op: 
    '-' {
        $$ = '-';
    }
    | '!' {
	$$ = '!';
    }
    | '~' {
	$$ = '~';
    }
;

%%

int yyerror(char *s)
{
	fprintf(stderr, "error: %s\n", s);
	return 0;
}
