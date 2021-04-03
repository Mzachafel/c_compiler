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
"," { return COMMA; }
"=" { return A; }
"+=" { return ADDA; }
"-=" { return SUBA; }
"*=" { return MULA; }
"/=" { return DIVA; }
"%=" { return MODA; }
"<<=" { return SHLA; }
">>=" { return SHRA; }
"|=" { return BORA; }
"^=" { return BXORA; }
"&=" { return BANDA; }
"?" { return TERN; }
":" { return ':'; }
"||" { return OR; }
"&&" { return AND; }
"|" { return BOR; }
"^" { return BXOR; }
"&" { return BAND; }
"==" { return E; }
"!=" { return NE; }
"<" { return L; }
"<=" { return LE; }
">" { return G; }
">=" { return GE; }
"<<" { return SHL; }
">>" { return SHR; }
"+" { return ADD; }
"-" { return SUB; }
"*" { return MUL; }
"/" { return DIV; }
"%" { return MOD; }
"~" { return NOT; }
"!" { return LNEG; }
"++" { return INC; }
"--" { return DEC; }
"int" { yylval.type = strdup(yytext); return TYPE; }
"if" { return IF; }
"else" { return ELSE; }
"for" { return FOR; }
"while" { return WHILE; }
"do" { return DO; }
"break" { return BREAK; }
"continue" { return CONTINUE; }
"return" { return RETURN; }
[a-zA-Z_][a-zA-Z0-9_]* { yylval.identifier = strdup(yytext); return IDENTIFIER; }
[0-9]+ { yylval.constant = atoi(yytext); return CONSTANT; }
[ \t\n] { /* skip whitespaces */ }
. { return ERROR; }

%%
