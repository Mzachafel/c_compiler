#include <stdio.h>
#include <stdlib.h>
#include "parser.tab.h"

struct statement {
	int act;
	int val;
};

struct statement *creatstmt(int act, int val)
{
	struct statement *stmt = (struct statement *) 
		           malloc(sizeof(struct statement));
	stmt->act = act;
	stmt->val = val;

	return stmt;
}

void writestmt(struct statement *stmt, FILE *outfile)
{
	switch (stmt->act) {
	case RET:
		fprintf(outfile, "\tmovl    $%d, %%eax\n", stmt->val);
		fprintf(outfile, "\tret\n");
		break;
	}
}

void clearstmt(struct statement *stmt)
{
	free(stmt);
}



struct body {
	struct statement *stmts[10];
	int currstmt;
};

struct body *creatbdy(struct statement *stmt)
{
	struct body *bdy = (struct body *)
		               malloc(sizeof(struct body));
	bdy->currstmt = 0;
	bdy->stmts[bdy->currstmt++] = stmt;

	return bdy;
}

struct body *addstmt(struct body *bdy, struct statement *stmt)
{
	if (bdy->currstmt < 10)
		bdy->stmts[bdy->currstmt++] = stmt;

	return bdy;
}

void writebdy(struct body *bdy, FILE *outfile)
{
	for (int i=0; i<bdy->currstmt; i++)
		writestmt(bdy->stmts[i], outfile);
}

void clearbdy(struct body *bdy)
{
	for (int i=0; i<bdy->currstmt; i++)
		clearstmt(bdy->stmts[i]);
	free(bdy);
}



struct function {
	struct body *bdy;
	int ret;
	char *id;
};

struct function *creatfunc(struct body *bdy, int ret, char* id)
{
	struct function *func = (struct function *) malloc(sizeof(struct function));
	func->bdy = bdy;
	func->ret = ret;
	func->id = id;

	return func;
}

void writefunc(struct function *func, FILE *outfile)
{
	fprintf(outfile, "\t.globl  %s\n", func->id);
	fprintf(outfile, "%s:\n", func->id);
	writebdy(func->bdy, outfile);
}

void clearfunc(struct function *func)
{
	clearbdy(func->bdy);
	free(func->id);
	free(func);
}
