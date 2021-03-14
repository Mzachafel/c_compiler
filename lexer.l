%{
#include "parser.tab.h"
#include <string.h>
%}

%%

"{" { return LBRACE; }
"}" { return RBRACE; }
"(" { return LPAREN; }
")" { return RPAREN; }
";" { return SEMICOL; }
"int" { return INT; }
"return" { return RET; }
[a-zA-Z][a-zA-Z0-9-]* { yylval.id = strdup(yytext); return ID; }
[0-9]+ { yylval.token = atoi(yytext); return INTLIT; }
[ \t\n] { /* skip whitespaces */ }
. { return UNDEFINED; }

%%
