#ifndef MY_AST_H
#define MY_AST_H

union trmval;

struct term;

struct term *creattrm(union trmval, int);
void writetrm(struct term *, FILE *);
void cleartrm(struct term *);

struct factor;

struct factor *creatfctr(struct factor *, int, struct term *);
void writefctr(struct factor *, FILE *);
void clearfctr(struct factor *);

struct expression;

struct expression *createxpr(struct expression *, int, struct factor *);
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
