%{

#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

extern int yylex();
extern int yyerror(char *s);
extern FILE *outfile;
union lval {
	struct expression *expr;
	int num;
} val;

%}

%union {
	struct function *func;
	struct body *bdy;
	struct statement *stmt;
	struct expression *expr;
	char *type;
	char *identifier;
	int number;
	char unop;
}

%token <identifier> IDENTIFIER
%token <type> TYPE
%token <number> NUMBER 
%token AND OR SHL SHR E NE L LE G GE
%token RETURN ERROR

%type <func> function
%type <bdy> body
%type <stmt> statement
%type <expr> logic_or_expr logic_and_expr
             bit_or_expr bit_xor_expr bit_and_expr
             equality_expr relational_expr shift_expr
             additive_expr factor term
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
	 RETURN logic_or_expr ';' { 
	     $$ = creatstmt($2, 0);
         }
;

logic_or_expr:
	     logic_and_expr
	     | logic_or_expr OR logic_and_expr {
		 val.expr = $1;
	         $$ = createxpr(val, OR, $3);
	     }
;

logic_and_expr:
	      bit_or_expr
	      | logic_and_expr AND bit_or_expr {
		  val.expr = $1;
	          $$ = createxpr(val, AND, $3);
	      }
;

bit_or_expr:
	   bit_xor_expr
	   | bit_or_expr '|' bit_xor_expr {
	       val.expr = $1;
	       $$ = createxpr(val, '|', $3);
	   }

bit_xor_expr:
	   bit_and_expr
	   | bit_xor_expr '^' bit_and_expr {
	       val.expr = $1;
	       $$ = createxpr(val, '^', $3);
	   }

bit_and_expr:
	   equality_expr
	   | bit_and_expr '&' equality_expr {
	       val.expr = $1;
	       $$ = createxpr(val, '&', $3);
	   }

equality_expr:
	     relational_expr
	     | equality_expr E relational_expr {
		 val.expr = $1;
	         $$ = createxpr(val, E, $3);
	     }
	     | equality_expr NE relational_expr {
		 val.expr = $1;
	         $$ = createxpr(val, NE, $3);
	     }
;

relational_expr:
	       shift_expr
	       | relational_expr L shift_expr {
		   val.expr = $1;
	           $$ = createxpr(val, L, $3);
	       }
	       | relational_expr LE shift_expr {
		   val.expr = $1;
	           $$ = createxpr(val, LE, $3);
	       }
	       | relational_expr G shift_expr {
		   val.expr = $1;
	           $$ = createxpr(val, G, $3);
	       }
	       | relational_expr GE shift_expr {
		   val.expr = $1;
	           $$ = createxpr(val, GE, $3);
	       }
;

shift_expr:
	  additive_expr
	  | shift_expr SHL additive_expr {
	      val.expr = $1;
	      $$ = createxpr(val, SHL, $3);
	  }
	  | shift_expr SHR additive_expr {
	      val.expr = $1;
	      $$ = createxpr(val, SHR, $3);
	  }

additive_expr: 
	     factor
	     | additive_expr '+' factor {
		 val.expr = $1;
	         $$ = createxpr(val, '+', $3);
	     }
	     | additive_expr '-' factor {
		 val.expr = $1;
	         $$ = createxpr(val, '-', $3);
	     }
;

factor:
      term
      | factor '*' term {
	  val.expr = $1;
          $$ = createxpr(val, '*', $3);
      }
      | factor '/' term {
	  val.expr = $1;
          $$ = createxpr(val, '/', $3);
      }
      | factor '%' term {
	  val.expr = $1;
          $$ = createxpr(val, '%', $3);
      }
;

term: 
    NUMBER {
	val.num = $1;
        $$ = createxpr(val, NUMBER, 0);
    }
    | unary_op term {
	val.expr = $2;
        $$ = createxpr(val, $1, 0);
    }
    | '(' logic_or_expr ')' {
        $$ = $2;
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
