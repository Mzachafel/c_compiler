#ifndef MY_AST_H
#define MY_AST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"
#include "util.h"

struct expression;

struct expression *creatternexpr(struct expression *, int, struct expression *, struct expression *);
struct expression *creatbinexpr(struct expression *, int, struct expression *);
struct expression *creatunexpr(struct expression *, int);
struct expression *creatvarexpr(char *, int, struct expression *);
struct expression *creatconstexpr(int, int);
void writeexpr(struct expression *, FILE *);
void clearexpr(struct expression *);

enum stmttokens {
	EXPR, SCOPE, COND, RET, DECL
};

struct statement;

struct statement *creatdefstmt(int, struct expression *);
struct statement *creatscopestmt(int, struct body *);
struct statement *creatcondstmt(int, struct expression *, struct statement *, struct statement *);
struct statement *creatdeclstmt(int, struct expression *, char *);
void writestmt(struct statement *, FILE *);
int isreturn(struct statement *);
void clearstmt(struct statement *);

struct body;

struct body *creatbdy(void);
struct body *addstmt(struct body *, struct statement *);
void writebdy(struct body *, FILE *);
void clearbdy(struct body *);

struct function;

struct function *creatfunc(char *, struct body *);
void writefunc(struct function *, FILE *);
void clearfunc(struct function *);

#endif
