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
"int" { return TYPE; }
"return" { return RETURN; }
[a-zA-Z][a-zA-Z0-9-]* { yylval.identifier = strdup(yytext); return IDENTIFIER; }
[0-9]+ { yylval.token = atoi(yytext); return NUMBER; }
[ \t\n] { /* skip whitespaces */ }
. { return ERROR; }

%%
