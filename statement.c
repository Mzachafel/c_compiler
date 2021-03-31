#include "ast.h"

struct statement {
	int act;
	struct expression *expr;
	union additionalvalue{
		char *name;
		struct statement **stmts;
		struct body *bdy;
	} addtval;
};

struct statement *creatdefstmt(int act, struct expression *expr)
{
	struct statement *stmt = (struct statement *) malloc(sizeof(struct statement));
	stmt->act = act;
	stmt->expr = expr;

	return stmt;
}

struct statement *creatscopestmt(int act, struct body *bdy)
{
	struct statement *stmt = (struct statement *) malloc(sizeof(struct statement));
	stmt->act = act;
	stmt->expr = NULL;
	stmt->addtval.bdy = bdy;

	return stmt;
}

struct statement *creatcondstmt(int act, struct expression *expr, struct statement *stmt1, struct statement *stmt2)
{
	struct statement *stmt = (struct statement *) malloc(sizeof(struct statement));
	stmt->act = act;
	stmt->expr = expr;
	stmt->addtval.stmts = (struct statement **) calloc(2, sizeof(struct statement *));
	stmt->addtval.stmts[0] = stmt1;
	stmt->addtval.stmts[1] = stmt2;

	return stmt;
}

struct statement *creatdeclstmt(int act, struct expression *expr, char *name)
{
	struct statement *stmt = (struct statement *) malloc(sizeof(struct statement));
	stmt->act = act;
	stmt->expr = expr;
	stmt->addtval.name = strdup(name);

	return stmt;
}

void writestmt(struct statement *stmt, FILE *outfile)
{
	char *label1, *label2;
	switch (stmt->act) {
	case EXPR:
		writeexpr(stmt->expr, outfile);
		break;
	case SCOPE:
		enterscope();
		writebdy(stmt->addtval.bdy, outfile);
		exitscope(outfile);
		break;
	case COND:
		writeexpr(stmt->expr, outfile);
		fprintf(outfile, "\tcmp     $0,%%rax\n");
		label1 = newlabel();
		fprintf(outfile, "\tje      %s\n", label1);
		writestmt(stmt->addtval.stmts[0], outfile);
		if (stmt->addtval.stmts[1] == NULL) {
			fprintf(outfile, "%s:\n", label1);
			free(label1);
		} else {
			label2 = newlabel();
			fprintf(outfile, "\tjmp     %s\n", label2);
			fprintf(outfile, "%s:\n", label1);
			free(label1);
			writestmt(stmt->addtval.stmts[1], outfile);
			fprintf(outfile, "%s:\n", label2);
			free(label2);
		}
		break;
	case RET:
		writeexpr(stmt->expr, outfile);	
		fprintf(outfile, "\tmov     %%rbp,%%rsp\n");
		fprintf(outfile, "\tpop     %%rbp\n");
		fprintf(outfile, "\tret\n");
		break;
	case DECL:
		if (stmt->expr == NULL)
			fprintf(outfile, "\tmov     $0,%%rax\n");
		else
			writeexpr(stmt->expr, outfile);
		fprintf(outfile, "\tpush    %%rax\n");
		pushvar(stmt->addtval.name);
		break;
	}
}

void clearstmt(struct statement *stmt)
{
	if (stmt->expr)
		clearexpr(stmt->expr);
	if (stmt->act == SCOPE)
		clearbdy(stmt->addtval.bdy);
	else if (stmt->act == COND) {
		clearstmt(stmt->addtval.stmts[0]);
		if (stmt->addtval.stmts[1])
			clearstmt(stmt->addtval.stmts[1]);
		free(stmt->addtval.stmts);
	} else if (stmt->act == DECL)
		free(stmt->addtval.name);
	free(stmt);
}

