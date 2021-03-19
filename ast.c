#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"

char *newlabel(void)
{
	static int counter = 1;
	if (counter > 230)
		counter = 1;
	int tmp = counter, i = 0;
	char *label = (char *) malloc(10*sizeof(char));
	for (i=0; tmp; i++, tmp/=26)
		label[i] = tmp%26 + 'a' - 1;
	label[i] = '\0';
	counter++;

	return label;
}


union lval {
	struct expression *expr;
	int num;
};

struct expression {
	union lval lexpr;
	int act;
	struct expression *rexpr;
};

struct expression *createxpr(union lval lexpr, int act, struct expression *rexpr)
{
	struct expression *expr = (struct expression *) malloc(sizeof(struct expression));
	if (!act)
		expr->lexpr.num = lexpr.num;
	else
		expr->lexpr.expr = lexpr.expr;
	expr->act = act;
	expr->rexpr = rexpr;

	return expr;
}

void writeexpr(struct expression *expr, FILE *outfile)
{
	if (expr->act == NUMBER)
		fprintf(outfile, "\tmov     $%d,%%rax\n", expr->lexpr.num);
	else if (expr->rexpr == NULL) { /* unary operator */
		writeexpr(expr->lexpr.expr, outfile);
		switch (expr->act) {
		case '-': /* negotiation:
                	     x -> -x */
			fprintf(outfile, "\tneg     %%rax\n");
			break;
		case '!': /* logical negotiation:
	        	     0 -> 1, anything else -> 0 */
			fprintf(outfile, "\tcmp     $0,%%rax\n");
			fprintf(outfile, "\tsete    %%al\n");
			break;
		case '~': /* bitwise complement:
		             0110 -> 1001 */
			fprintf(outfile, "\tnot     %%rax\n");
			break;
		}
	}
	else { /* binary operator */
		writeexpr(expr->lexpr.expr, outfile);
		char *label1, *label2;
		switch(expr->act) {
		case OR:
			fprintf(outfile, "\tcmp     $0,%%rax\n");
			label1 = newlabel();
			fprintf(outfile, "\tje      %s\n", label1);
			fprintf(outfile, "\tmov     $1,%%rax\n");
			label2 = newlabel();
			fprintf(outfile, "\tjmp     %s\n", label2);
			fprintf(outfile, "%s:\n", label1);
			free(label1);
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tcmp     $0,%%rax\n");
			fprintf(outfile, "\tsetne   %%al\n");
			fprintf(outfile, "%s:\n", label2);
			free(label2);
			break;
		case AND:
			fprintf(outfile, "\tcmp     $0,%%rax\n");
			label1 = newlabel();
			fprintf(outfile, "\tjne     %s\n", label1);
			fprintf(outfile, "\tmov     $0,%%rax\n");
			label2 = newlabel();
			fprintf(outfile, "\tjmp     %s\n", label2);
			fprintf(outfile, "%s:\n", label1);
			free(label1);
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tcmp     $0,%%rax\n");
			fprintf(outfile, "\tsetne   %%al\n");
			fprintf(outfile, "%s:\n", label2);
			free(label2);
			break;
		case '|':
			fprintf(outfile, "\tpush    %%rax\n");
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tpop     %%rcx\n");
			fprintf(outfile, "\tor      %%rcx,%%rax\n");
			break;
		case '^':
			fprintf(outfile, "\tpush    %%rax\n");
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tpop     %%rcx\n");
			fprintf(outfile, "\txor     %%rcx,%%rax\n");
			break;
		case '&':
			fprintf(outfile, "\tpush    %%rax\n");
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tpop     %%rcx\n");
			fprintf(outfile, "\tand     %%rcx,%%rax\n");
			break;
		case E:
			fprintf(outfile, "\tpush    %%rax\n");
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tpop     %%rcx\n");
			fprintf(outfile, "\tcmp     %%rcx,%%rax\n");
			fprintf(outfile, "\tsete    %%al\n");
			break;
		case NE:
			fprintf(outfile, "\tpush    %%rax\n");
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tpop     %%rcx\n");
			fprintf(outfile, "\tcmp     %%rcx,%%rax\n");
			fprintf(outfile, "\tsetne    %%al\n");
			break;
		case L:
			fprintf(outfile, "\tpush    %%rax\n");
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tpop     %%rcx\n");
			fprintf(outfile, "\tcmp     %%rcx,%%rax\n");
			fprintf(outfile, "\tsetg    %%al\n");
			break;
		case LE:
			fprintf(outfile, "\tpush    %%rax\n");
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tpop     %%rcx\n");
			fprintf(outfile, "\tcmp     %%rcx,%%rax\n");
			fprintf(outfile, "\tsetge   %%al\n");
			break;
		case G:
			fprintf(outfile, "\tpush    %%rax\n");
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tpop     %%rcx\n");
			fprintf(outfile, "\tcmp     %%rcx,%%rax\n");
			fprintf(outfile, "\tsetl    %%al\n");
			break;
		case GE:
			fprintf(outfile, "\tpush    %%rax\n");
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tpop     %%rcx\n");
			fprintf(outfile, "\tcmp     %%rcx,%%rax\n");
			fprintf(outfile, "\tsetle   %%al\n");
			break;
		case SHL:
			fprintf(outfile, "\tpush    %%rax\n");
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tmov     %%al,%%cl\n");
			fprintf(outfile, "\tpop     %%rax\n");
			fprintf(outfile, "\tshl     %%cl,%%rax\n");
			break;
		case SHR:
			fprintf(outfile, "\tpush    %%rax\n");
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tmov     %%al,%%cl\n");
			fprintf(outfile, "\tpop     %%rax\n");
			fprintf(outfile, "\tshr     %%cl,%%rax\n");
			break;
		case '+':
			fprintf(outfile, "\tpush    %%rax\n");
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tpop     %%rcx\n");
			fprintf(outfile, "\tadd     %%rcx,%%rax\n");
			break;
		case '-':
			fprintf(outfile, "\tpush    %%rax\n");
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tpop     %%rcx\n");
			fprintf(outfile, "\tsub     %%rax,%%rcx\n");
			fprintf(outfile, "\tmov     %%rcx,%%rax\n");
			break;
		case '*':
			fprintf(outfile, "\tpush    %%rax\n");
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tpop     %%rcx\n");
			fprintf(outfile, "\tmul     %%rcx\n");
			break;
		case '/':
			fprintf(outfile, "\tpush    %%rax\n");
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tmov     %%rax,%%rcx\n");
			fprintf(outfile, "\tpop     %%rax\n");
			fprintf(outfile, "\tmov     $0,%%rdx\n");
			fprintf(outfile, "\tdiv     %%rcx\n");
			break;
		case '%':
			fprintf(outfile, "\tpush    %%rax\n");
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tmov     %%rax,%%rcx\n");
			fprintf(outfile, "\tpop     %%rax\n");
			fprintf(outfile, "\tmov     $0,%%rdx\n");
			fprintf(outfile, "\tdiv     %%rcx\n");
			fprintf(outfile, "\tmov     %%rdx,%%rax\n");
			break;
		}
	}
}

void clearexpr(struct expression *expr)
{
	if (expr->act != NUMBER)
		clearexpr(expr->lexpr.expr);
	if (expr->rexpr)
		clearexpr(expr->rexpr);
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
