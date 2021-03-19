#ifndef MY_AST_H
#define MY_AST_H

union lval;

struct expression;

struct expression *createxpr(union lval, int, struct expression *);
void writeexpr(struct expression *, FILE *);
void clearexpr(struct expression *);

struct statement;

struct statement *creatstmt(struct expression *, int);
void writestmt(struct statement *, FILE *);
void clearstmt(struct statement *);

struct body;

struct body *creatbdy(struct statement *);
struct body *addstmt(struct body *, struct statement *);
void writebdy(struct body *, FILE *);
void clearbdy(struct body *);

struct function;

struct function *creatfunc(char *, char *, struct body *);
void writefunc(struct function *, FILE *);
void clearfunc(struct function *);

#endif
