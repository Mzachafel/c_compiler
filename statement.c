#include "ast.h"

struct statement {
	char *name;
	struct expression *expr;
	int act;
};

struct statement *creatstmt(char *name, struct expression *expr, int act)
{
	struct statement *stmt = (struct statement *) malloc(sizeof(struct statement));
	stmt->name = (name) ? strdup(name) : NULL;
	stmt->expr = expr;
	stmt->act = act;

	return stmt;
}

void writestmt(struct statement *stmt, FILE *outfile)
{
	switch (stmt->act) {
	case EXPR:
		writeexpr(stmt->expr, outfile);
		break;
	case DECL:
		fprintf(outfile, "\tmov     $0,%%rax\n");
		fprintf(outfile, "\tpush    %%rax\n");
		pushvar(stmt->name);
		break;
	case ASGN:
		writeexpr(stmt->expr, outfile);
		fprintf(outfile, "\tpush    %%rax\n");
		pushvar(stmt->name);
		break;
	case RET:
		writeexpr(stmt->expr, outfile);	
		fprintf(outfile, "\tmov     %%rbp,%%rsp\n");
		fprintf(outfile, "\tpop     %%rbp\n");
		fprintf(outfile, "\tret\n");
		break;
	}
}

int isreturn(struct statement *stmt)
{
	if (stmt->act == 3)
		return 1;
	return 0;
}

void clearstmt(struct statement *stmt)
{
	if (stmt->name)
		free(stmt->name);
	if (stmt->expr)
		clearexpr(stmt->expr);
	free(stmt);
}

