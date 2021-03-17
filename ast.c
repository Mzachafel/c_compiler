#include <stdio.h>
#include <stdlib.h>
#include <string.h>

union trmval {
	struct expression *expr;
	struct term *trm;
	int num;
};

struct term {
	union trmval val;
	int act;
};

struct expression;
struct term *creattrm(union trmval val, int act)
{
	struct term *trm = (struct term *) malloc(sizeof(struct term));
	trm->act = act;
	switch (act) {
	case 1:
		trm->val.expr = val.expr;
		break;
	case '-':
	case '!':
	case '~':
		trm->val.trm = val.trm;
		break;
	default:
		trm->val.num = val.num;
	}

	return trm;
}

void writeexpr(struct expression *, FILE *);
void writetrm(struct term *trm, FILE *outfile)
{
	switch (trm->act) {
	case 1:
		writeexpr(trm->val.expr, outfile);
		break;
	case '-': /* negotiation:
                     x -> -x */
		writetrm(trm->val.trm, outfile);
		fprintf(outfile, "\tneg     %%rax\n");
		break;
	case '!': /* logical negotiation:
	             0 -> 1, anything else -> 0 */
		writetrm(trm->val.trm, outfile);
		fprintf(outfile, "\tcmp     $0,%%rax\n");
		fprintf(outfile, "\tmov     $0,%%rax\n");
		fprintf(outfile, "\tsete    %%rax\n");
		break;
	case '~': /* bitwise complement:
	             0110 -> 1001 */
		writetrm(trm->val.trm, outfile);
		fprintf(outfile, "\tnot     %%rax\n");
		break;
	default: /* new value */
		fprintf(outfile, "\tmov     $%d,%%rax\n", trm->val.num);
	}
}

void clearexpr(struct expression *);
void cleartrm(struct term *trm)
{
	switch (trm->act) {
	case 1:
		clearexpr(trm->val.expr);
		break;
	case '-':
	case '!':
	case '~':
		cleartrm(trm->val.trm);
		break;
	}
	free(trm);
}



struct factor {
	struct factor *lfctr;
	struct term *rtrm;
	int act;
};

struct factor *creatfctr(struct factor *lfctr, int act, struct term *rtrm)
{
	struct factor *fctr = (struct factor *) malloc(sizeof(struct factor));
	fctr->lfctr = lfctr;
	fctr->act = act;
	fctr->rtrm = rtrm;

	return fctr;
}

void writefctr(struct factor *fctr, FILE *outfile)
{
	switch(fctr->act) {
	case '*':
		writefctr(fctr->lfctr, outfile);
		fprintf(outfile, "\tpush    %%rax\n");
		writetrm(fctr->rtrm, outfile);
		fprintf(outfile, "\tpop     %%rcx\n");
		fprintf(outfile, "\tmul     %%rcx\n");
		break;
	case '/':
		writefctr(fctr->lfctr, outfile);
		fprintf(outfile, "\tpush    %%rax\n");
		writetrm(fctr->rtrm, outfile);
		fprintf(outfile, "\tpop     %%rcx\n");
		fprintf(outfile, "\tdiv     %%rcx\n");
		break;
	default:
		writetrm(fctr->rtrm, outfile);
	}
}

void clearfctr(struct factor *fctr)
{
	cleartrm(fctr->rtrm);
	if (fctr->act)
		clearfctr(fctr->lfctr);
	free(fctr);
}



struct expression {
	struct expression *lexpr;
	struct factor *rfctr;
	int act;
};

struct expression *createxpr(struct expression *lexpr, int act, struct factor *rfctr)
{
	struct expression *expr = (struct expression *) malloc(sizeof(struct expression));
	expr->lexpr = lexpr;
	expr->act = act;
	expr->rfctr = rfctr;

	return expr;
}

void writeexpr(struct expression *expr, FILE *outfile)
{
	switch(expr->act) {
	case '+':
		writeexpr(expr->lexpr, outfile);
		fprintf(outfile, "\tpush    %%rax\n");
		writefctr(expr->rfctr, outfile);
		fprintf(outfile, "\tpop     %%rcx\n");
		fprintf(outfile, "\tadd     %%rax,%%rcx\n");
		break;
	case '-':
		writeexpr(expr->lexpr, outfile);
		fprintf(outfile, "\tpush    %%rax\n");
		writefctr(expr->rfctr, outfile);
		fprintf(outfile, "\tpop     %%rcx\n");
		fprintf(outfile, "\tsub     %%rax,%%rcx\n");
		break;
	default:
		writefctr(expr->rfctr, outfile);
	}
}

void clearexpr(struct expression *expr)
{
	clearfctr(expr->rfctr);
	if (expr->act)
		clearexpr(expr->lexpr);
	free(expr);
}



struct statement {
	struct expression *expr;
	int act;
};

struct statement *creatstmt(struct expression* expr, int act)
{
	struct statement *stmt = (struct statement *) malloc(sizeof(struct statement));
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
	int curstmt;
};

struct body *creatbdy(struct statement *stmt)
{
	struct body *bdy = (struct body *) malloc(sizeof(struct body));
	bdy->curstmt = 0;
	bdy->stmts[bdy->curstmt++] = stmt;

	return bdy;
}

struct body *addstmt(struct body *bdy, struct statement *stmt)
{
	if (bdy->curstmt < 10)
		bdy->stmts[bdy->curstmt++] = stmt;

	return bdy;
}

void writebdy(struct body *bdy, FILE *outfile)
{
	for (int i=0; i<bdy->curstmt; i++)
		writestmt(bdy->stmts[i], outfile);
}

void clearbdy(struct body *bdy)
{
	for (int i=0; i<bdy->curstmt; i++)
		clearstmt(bdy->stmts[i]);
	free(bdy);
}



struct function {
	struct body *bdy;
	char *type;
	char *id;
};

struct function *creatfunc(char *type, char *id, struct body *bdy)
{
	struct function *func = (struct function *) malloc(sizeof(struct function));
	func->type = strdup(type);
	func->id = strdup(id);
	func->bdy = bdy;

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
