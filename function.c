#include "ast.h"

struct function {
	char *name;
	struct body *params;
	struct body *bdy;
};

struct function *creatfunc(char *name, struct body *params, struct body *bdy)
{
	struct function *func = (struct function *) malloc(sizeof(struct function));
	func->name = strdup(name);
	func->params = params;
	func->bdy = bdy;

	return func;
}

void writefunc(struct function *func, FILE *outfile)
{
	if (func->params)
		pushfunc(func->name, getparams(func->params));
	else
		pushfunc(func->name, 0);
	if (func->bdy != NULL) {
		if (!checkfuncname(func->name)) {
			fprintf(stderr, "Function %s can't be defined twice\n", func->name);
			exit(1);
		}
		enterscope();
		if (func->params)
			pushparams(func->params);
		fprintf(outfile, "\t.globl  %s\n", func->name);
		fprintf(outfile, "%s:\n", func->name);
		fprintf(outfile, "\tpush    %%rbp\n");
		fprintf(outfile, "\tmov     %%rsp,%%rbp\n");
		if (func->bdy)
			writebdy(func->bdy, outfile);
		fprintf(outfile, "\tmov     $0,%%rax\n");
		fprintf(outfile, "\tmov     %%rbp,%%rsp\n");
		fprintf(outfile, "\tpop     %%rbp\n");
		fprintf(outfile, "\tret\n");
		exitscope(outfile);
	}
}

void clearfunc(struct function *func)
{
	free(func->name);
	clearbdy(func->params);
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
	if (stmt == NULL)
		return bdy;
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
}

int getparams(struct body *params)
{
	return params->curstmt;
}

void pushparams(struct body *params)
{
	for (int i = params->curstmt-1; i >= 0; i--)
		pushparam(params->stmts[i], i);
}

void clearbdy(struct body *bdy)
{
	if (!bdy)
		return;
	for (int i=0; i<bdy->curstmt; i++)
		clearstmt(bdy->stmts[i]);
	free(bdy->stmts);
	free(bdy);
}

