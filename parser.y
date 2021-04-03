%{

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
	char *type;
	char *identifier;
	int constant;
	int token;
}

%token <identifier> IDENTIFIER
%token <type> TYPE
%token <constant> CONSTANT 
%token COMMA
       A ADDA SUBA MULA DIVA MODA SHLA SHRA BORA BXORA BANDA
       TERN OR AND BOR BXOR BAND
       E NE L LE G GE
       SHL SHR ADD SUB MUL DIV MOD
       NOT LNEG NEG INC DEC PREFINC PREFDEC POSTINC POSTDEC
%token IF ELSE FOR WHILE DO BREAK CONTINUE RETURN ERROR

%type <func> function
%type <bdy> body
%type <stmt> block_item statement declaration
%type <expr> exp_opt_semicol exp_opt_paren
             expressions expression
             conditional_expr
	     logic_or_expr logic_and_expr
             bit_or_expr bit_xor_expr bit_and_expr
             equality_expr relational_expr shift_expr
             additive_expr multiplicative_expr
	     pref_expr post_expr
%type <token> compound_asgn

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
	    $$ = creatfunc($2, $6); 
        }
;

body: 
    block_item {
        struct body *bdy = creatbdy();
	$$ = addstmt(bdy, $1);
    }
    | body block_item {
        $$ = addstmt($1, $2);
    }
;

block_item:
	  statement { $$ = $1; }
	  | declaration { $$ = $1; }
;

statement:
	 /* any expression */
	 expressions ';' {
	     $$ = creatdefstmt(EXPR, $1);
	 }
	 /* new scope */
	 | '{' body '}' {
	     $$ = creatscopestmt(SCOPE, $2);
	 }
	 | IF '(' expressions ')' statement {
	     $$ = creatcondstmt(COND, $3, $5, NULL);
	 }
	 | IF '(' expressions ')' statement ELSE statement {
	     $$ = creatcondstmt(COND, $3, $5, $7);
	 }
	 | FOR '(' exp_opt_semicol exp_opt_semicol exp_opt_paren statement {
	     $$ = creatforexprstmt(FOR, EXPR, $3, $4, $5, $6);
	 }
	 | FOR '(' declaration exp_opt_semicol exp_opt_paren statement {
	     $$ = creatfordeclstmt(FOR, DECL, $3, $4, $5, $6);
	 }
	 | WHILE '(' expressions ')' statement {
	     $$ = creatwhilestmt(WHILE, $3, $5);
	 }
	 | DO statement WHILE '(' expressions ')' ';' {
	     $$ = creatwhilestmt(DO, $5, $2);
	 }
	 | BREAK ';' {
	     $$ = creatdefstmt(BREAK, NULL);
	 }
	 | CONTINUE ';' {
	     $$ = creatdefstmt(CONTINUE, NULL);
	 }
	 /* return statement */
	 | RETURN expressions ';' { 
	     $$ = creatdefstmt(RET, $2);
         }
	 /* empty statement */
	 | ';' { $$ = NULL; }
	 /* empty scope */
	 | '{' '}' { $$ = NULL; }	     
;

exp_opt_semicol:
	       expressions ';' {
	           $$ = $1;
	       }
	       | ';' {
	           $$ = NULL;
	       }
;

exp_opt_paren:
	     expressions ')' {
	         $$ = $1;
	     }
	     | ')' {
	         $$ = NULL;
	     }
;

declaration:
	   /* declaration */
	   TYPE IDENTIFIER ';' {
	       $$ = creatdeclstmt(DECL, $2, NULL);
	   }
	   /* declaration and initialization */
	   | TYPE IDENTIFIER A expressions ';' {
	       $$ = creatdeclstmt(DECL, $2, $4);
	   }
;

expressions:
	   expression
	   | expressions COMMA expression {
	       $$ = creatbinexpr($1, COMMA, $3);
	   }
;

expression:
	  conditional_expr {
	      $$ = $1;
	  }
	  | IDENTIFIER compound_asgn expression {
	      $$ = creatvarexpr($1, $2, $3);
	  }
;

compound_asgn:
	     A { $$ = A; }
	     | ADDA { $$ = ADDA; }
	     | SUBA { $$ = SUBA; }
	     | MULA { $$ = MULA; }
	     | DIVA { $$ = DIVA; }
	     | MODA { $$ = MODA; }
	     | SHLA { $$ = SHLA; }
	     | SHRA { $$ = SHRA; }
	     | BORA { $$ = BORA; }
	     | BXORA { $$ = BXORA; }
	     | BANDA { $$ = BANDA; }
;

conditional_expr:
		logic_or_expr
		| logic_or_expr TERN expressions ':' conditional_expr {
		    $$ = creatternexpr($1, TERN, $3, $5);
		}
;

logic_or_expr:
	     logic_and_expr
	     | logic_or_expr OR logic_and_expr {
	         $$ = creatbinexpr($1, OR, $3);
	     }
;

logic_and_expr:
	      bit_or_expr
	      | logic_and_expr AND bit_or_expr {
	          $$ = creatbinexpr($1, AND, $3);
	      }
;

bit_or_expr:
	   bit_xor_expr
	   | bit_or_expr BOR bit_xor_expr {
	       $$ = creatbinexpr($1, BOR, $3);
	   }

bit_xor_expr:
	   bit_and_expr
	   | bit_xor_expr BXOR bit_and_expr {
	       $$ = creatbinexpr($1, BXOR, $3);
	   }

bit_and_expr:
	   equality_expr
	   | bit_and_expr BAND equality_expr {
	       $$ = creatbinexpr($1, BAND, $3);
	   }

equality_expr:
	     relational_expr
	     | equality_expr E relational_expr {
	         $$ = creatbinexpr($1, E, $3);
	     }
	     | equality_expr NE relational_expr {
	         $$ = creatbinexpr($1, NE, $3);
	     }
;

relational_expr:
	       shift_expr
	       | relational_expr L shift_expr {
	           $$ = creatbinexpr($1, L, $3);
	       }
	       | relational_expr LE shift_expr {
	           $$ = creatbinexpr($1, LE, $3);
	       }
	       | relational_expr G shift_expr {
	           $$ = creatbinexpr($1, G, $3);
	       }
	       | relational_expr GE shift_expr {
	           $$ = creatbinexpr($1, GE, $3);
	       }
;

shift_expr:
	  additive_expr
	  | shift_expr SHL additive_expr {
	      $$ = creatbinexpr($1, SHL, $3);
	  }
	  | shift_expr SHR additive_expr {
	      $$ = creatbinexpr($1, SHR, $3);
	  }

additive_expr: 
	     multiplicative_expr
	     | additive_expr ADD multiplicative_expr {
	         $$ = creatbinexpr($1, ADD, $3);
	     }
	     | additive_expr SUB multiplicative_expr {
	         $$ = creatbinexpr($1, SUB, $3);
	     }
;

multiplicative_expr:
      pref_expr
      | multiplicative_expr MUL pref_expr {
          $$ = creatbinexpr($1, MUL, $3);
      }
      | multiplicative_expr DIV pref_expr {
          $$ = creatbinexpr($1, DIV, $3);
      }
      | multiplicative_expr MOD pref_expr {
          $$ = creatbinexpr($1, MOD, $3);
      }
;

pref_expr:
	 post_expr
	 | SUB post_expr {
	     $$ = creatunexpr($2, NEG);
	 }
	 | LNEG post_expr {
	     $$ = creatunexpr($2, LNEG);
	 }
	 | NOT post_expr {
	     $$ = creatunexpr($2, NOT);
	 }
	 | INC IDENTIFIER {
	     $$ = creatvarexpr($2, PREFINC, NULL);
	 }
	 | DEC IDENTIFIER {
	     $$ = creatvarexpr($2, PREFDEC, NULL);
	 }
;

post_expr:
    CONSTANT {
	$$ = creatconstexpr($1, CONSTANT);
    }
    | IDENTIFIER {
        $$ = creatvarexpr($1, IDENTIFIER, NULL);
    }
    | IDENTIFIER INC {
        $$ = creatvarexpr($1, POSTINC, NULL);
    }
    | IDENTIFIER DEC {
        $$ = creatvarexpr($1, POSTDEC, NULL);
    }
    | '(' expression ')' {
	$$ = $2;
    }
;

%%

int yyerror(char *s)
{
	fprintf(stderr, "error: %s\n", s);
	return 0;
}
