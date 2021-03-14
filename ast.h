#ifndef MY_AST_H
#define MY_AST_H

struct statement;

struct statement *creatstmt(int, int);
void writestmt(struct statement *, FILE *);
void clearstmt(struct statement *);

struct body;

struct body *creatbdy(struct statement *);
struct body *addstmt(struct body *, struct statement *);
void writebdy(struct body *, FILE *);
void clearbdy(struct body *);

struct function;

struct function *creatfunc(struct body *, int, char *);
void writefunc(struct function *, FILE *);
void clearfunc(struct function *);

#endif
