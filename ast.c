#include <stdio.h>
#include <stdlib.h>

struct operation {
	int act;
	int val;
};

struct operation *creatop(int act, int val)
{
	struct operation *op = (struct operation *)
		           malloc(sizeof(struct operation));
	op->act = act;
	op->val = val;

	return op;
}

void writeop(struct operation *op, FILE *outfile)
{
	switch (op->act) {
	case 0:
		fprintf(outfile, "\tmov     $%d,%%eax\n", op->val);
		break;
	case '-':
		fprintf(outfile, "\tneg     %%eax\n");
		break;
	case '!':
		fprintf(outfile, "\tcmp     $0,%%eax\n");
		fprintf(outfile, "\tmov     $0,%%eax\n");
		fprintf(outfile, "\tsete    %%eax\n");
		break;
	case '~':
		fprintf(outfile, "\tnot     %%eax\n");
		break;
	}
}

void clearop(struct operation *op)
{
	free(op);
}



struct expression {
	struct operation *ops[10];
	int currop;
};

struct expression *createxpr(struct operation *op)
{
	struct expression *expr = (struct expression *) 
		            malloc(sizeof(struct expression));
	expr->currop = 0;
	expr->ops[expr->currop++] = op;

	return expr;
}

struct expression *addop(struct expression *expr, struct operation *op)
{
	if (expr->currop < 10)
		expr->ops[expr->currop++] = op;

	return expr;
}

void writeexpr(struct expression *expr, FILE *outfile)
{
	for (int i=0; i<expr->currop; i++)
		writeop(expr->ops[i], outfile);
}

void clearexpr(struct expression *expr)
{
	for (int i=0; i<expr->currop; i++)
		clearop(expr->ops[i]);
	free(expr);
}



struct statement {
	struct expression *expr;
	int act;
};

struct statement *creatstmt(struct expression* expr, int act)
{
	struct statement *stmt = (struct statement *) 
		           malloc(sizeof(struct statement));
	stmt->expr = expr;
	stmt->act = act;

	return stmt;
}

void writestmt(struct statement *stmt, FILE *outfile)
{
	switch (stmt->act) {
	case 0:
		writeexpr(stmt->expr, outfile);
		fprintf(outfile, "\tret\n");
		break;
	}
}

void clearstmt(struct statement *stmt)
{
	clearexpr(stmt->expr);
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
