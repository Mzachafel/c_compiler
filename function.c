#include "ast.h"

struct function {
	char *name;
	struct body *bdy;
};

struct function *creatfunc(char *name, struct body *bdy)
{
	struct function *func = (struct function *) malloc(sizeof(struct function));
	func->name = strdup(name);
	func->bdy = bdy;

	return func;
}

void writefunc(struct function *func, FILE *outfile)
{
	fprintf(outfile, "\t.globl  %s\n", func->name);
	fprintf(outfile, "%s:\n", func->name);
	fprintf(outfile, "\tpush    %%rbp\n");
	fprintf(outfile, "\tmov     %%rsp,%%rbp\n");
	writebdy(func->bdy, outfile);
}

void clearfunc(struct function *func)
{
	free(func->name);
	clearbdy(func->bdy);
	free(func);
}


#define DEFMAXSTMT 8
struct body {
	int maxstmt;
	struct statement **stmts;
	int curstmt;
};

struct body *creatbdy(void)
{
	struct body *bdy = (struct body *) malloc(sizeof(struct body));
	bdy->maxstmt = DEFMAXSTMT;
	bdy->stmts = (struct statement **) calloc(bdy->maxstmt, sizeof(struct statement *));
	bdy->curstmt = 0;

	return bdy;
}

struct body *addstmt(struct body *bdy, struct statement *stmt)
{
	if (bdy->curstmt == bdy->maxstmt) {
		bdy->maxstmt *= 2;
		bdy->stmts = realloc(bdy->stmts, bdy->maxstmt * sizeof(struct statement *));
	}
	bdy->stmts[bdy->curstmt++] = stmt;

	return bdy;
}

void writebdy(struct body *bdy, FILE *outfile)
{
	for (int i=0; i<bdy->curstmt; i++)
		writestmt(bdy->stmts[i], outfile);
	if (!isreturn(bdy->stmts[bdy->curstmt-1])) {
		fprintf(outfile, "\tmov     $0,%%rax\n");
		fprintf(outfile, "\tmov     %%rbp,%%rsp\n");
		fprintf(outfile, "\tpop     %%rbp\n");
		fprintf(outfile, "\tret\n");
	}
}

void clearbdy(struct body *bdy)
{
	for (int i=0; i<bdy->curstmt; i++)
		clearstmt(bdy->stmts[i]);
	free(bdy->stmts);
	free(bdy);
}

