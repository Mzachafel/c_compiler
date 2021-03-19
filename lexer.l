%{
#include "parser.tab.h"
#include <string.h>
%}

%%

"{" { return '{'; }
"}" { return '}'; }
"(" { return '('; }
")" { return ')'; }
";" { return ';'; }
"-" { return '-'; }
"~" { return '~'; }
"!" { return '!'; }
"+" { return '+'; }
"*" { return '*'; }
"/" { return '/'; }
"%" { return '%'; }
"&" { return '&'; }
"|" { return '|'; }
"^" { return '^'; }
"<<" { return SHL; }
">>" { return SHR; }
"&&" { return AND; }
"||" { return OR; }
"==" { return E; }
"!=" { return NE; }
"<" { return L; }
"<=" { return LE; }
">" { return G; }
">=" { return GE; }
"int" { yylval.type = strdup(yytext); return TYPE; }
"return" { return RETURN; }
[a-zA-Z][a-zA-Z0-9-]* { yylval.identifier = strdup(yytext); return IDENTIFIER; }
[0-9]+ { yylval.number = atoi(yytext); return NUMBER; }
[ \t\n] { /* skip whitespaces */ }
. { return ERROR; }

%%
