#include "ast.h"

struct conditionalstatement {
	struct expression *cond;
	struct statement *ifstmt;
	struct statement *elsestmt;
};

struct forstatement {
	int inittype;
	union initialization {
		struct expression *expr;
		struct statement *stmt;
	} init;
	struct expression *cond;
	struct expression *iter;
	struct statement *stmt;
};

struct whilestatement {
	struct expression *cond;
	struct statement *stmt;
};

struct variablestatement {
	char *name;
	struct expression *init;
};

struct statement {
	int act;
	union value {
		struct expression *expr;
		struct body *bdy;
		struct conditionalstatement *condstmt;
		struct forstatement *forstmt;
		struct whilestatement *whilestmt;
		struct variablestatement *varstmt;
	} val;
};

struct statement *creatdefstmt(int act, struct expression *expr)
{
	struct statement *stmt = (struct statement *) malloc(sizeof(struct statement));
	stmt->act = act;
	stmt->val.expr = expr;

	return stmt;
}

struct statement *creatscopestmt(int act, struct body *bdy)
{
	struct statement *stmt = (struct statement *) malloc(sizeof(struct statement));
	stmt->act = act;
	stmt->val.bdy = bdy;

	return stmt;
}

struct statement *creatcondstmt(int act, struct expression *expr, struct statement *stmt1, struct statement *stmt2)
{
	struct statement *stmt = (struct statement *) malloc(sizeof(struct statement));
	stmt->act = act;
	stmt->val.condstmt = (struct conditionalstatement *) malloc(sizeof(struct conditionalstatement));
	stmt->val.condstmt->cond = expr;
	stmt->val.condstmt->ifstmt = stmt1;
	stmt->val.condstmt->elsestmt = stmt2;

	return stmt;
}

struct statement *creatforexprstmt(int act, int inittype, struct expression *init, struct expression *cond,
		                   struct expression *iter, struct statement *stmt1)
{
	struct statement *stmt = (struct statement *) malloc(sizeof(struct statement));
	stmt->act = act;
	stmt->val.forstmt = (struct forstatement *) malloc(sizeof(struct forstatement));
	stmt->val.forstmt->inittype = inittype;
	stmt->val.forstmt->init.expr = init;
	stmt->val.forstmt->cond = cond;
	stmt->val.forstmt->iter = iter;
	stmt->val.forstmt->stmt = stmt1;

	return stmt;
}

struct statement *creatfordeclstmt(int act, int inittype, struct statement *init, struct expression *cond,
		                   struct expression *iter, struct statement *stmt1)
{
	struct statement *stmt = (struct statement *) malloc(sizeof(struct statement));
	stmt->act = act;
	stmt->val.forstmt = (struct forstatement *) malloc(sizeof(struct forstatement));
	stmt->val.forstmt->inittype = inittype;
	stmt->val.forstmt->init.stmt = init;
	stmt->val.forstmt->cond = cond;
	stmt->val.forstmt->iter = iter;
	stmt->val.forstmt->stmt = stmt1;

	return stmt;
}

struct statement *creatwhilestmt(int act, struct expression *cond, struct statement *stmt1)
{
	struct statement *stmt = (struct statement *) malloc(sizeof(struct statement));
	stmt->act = act;
	stmt->val.whilestmt = (struct whilestatement *) malloc(sizeof(struct whilestatement));
	stmt->val.whilestmt->cond = cond;
	stmt->val.whilestmt->stmt = stmt1;

	return stmt;
}

struct statement *creatdeclstmt(int act, char *name, struct expression *expr)
{
	struct statement *stmt = (struct statement *) malloc(sizeof(struct statement));
	stmt->act = act;
	stmt->val.varstmt = (struct variablestatement *) malloc(sizeof(struct variablestatement));
	stmt->val.varstmt->name = strdup(name);
	stmt->val.varstmt->init = expr;

	return stmt;
}

void writestmt(struct statement *stmt, FILE *outfile)
{
	if (stmt == NULL)
		return;
	char *label1, *label2;
	switch (stmt->act) {
	case EXPR:
		writeexpr(stmt->val.expr, outfile);
		break;
	case SCOPE:
		enterscope();
		writebdy(stmt->val.bdy, outfile);
		exitscope(outfile);
		break;
	case COND:
		writeexpr(stmt->val.condstmt->cond, outfile);
		fprintf(outfile, "\tcmp     $0,%%rax\n");
		label1 = newlabel();
		fprintf(outfile, "\tje      %s\n", label1);
		writestmt(stmt->val.condstmt->ifstmt, outfile);
		if (stmt->val.condstmt->elsestmt == NULL) {
			fprintf(outfile, "%s:\n", label1);
			free(label1);
		} else {
			label2 = newlabel();
			fprintf(outfile, "\tjmp     %s\n", label2);
			fprintf(outfile, "%s:\n", label1);
			free(label1);
			writestmt(stmt->val.condstmt->elsestmt, outfile);
			fprintf(outfile, "%s:\n", label2);
			free(label2);
		}
		break;
	case FOR:
		enterscope();
		pushlbl(newlabel(), newlabel());
		if (stmt->val.forstmt->inittype == EXPR && stmt->val.forstmt->init.expr)
			writeexpr(stmt->val.forstmt->init.expr, outfile);
		else
			writestmt(stmt->val.forstmt->init.stmt, outfile);
		label1 = newlabel();
		fprintf(outfile, "%s:\n", label1);
		if (stmt->val.forstmt->cond)
			writeexpr(stmt->val.forstmt->cond, outfile);
		else
			fprintf(outfile, "\tmov     $1,%%rax\n");
		fprintf(outfile, "\tcmp     $0,%%rax\n");
		fprintf(outfile, "\tje      %s\n", poplbl(1));
		writestmt(stmt->val.forstmt->stmt, outfile);
		fprintf(outfile, "%s:\n", poplbl(0));
		if (stmt->val.forstmt->iter)
			writeexpr(stmt->val.forstmt->iter, outfile);
		fprintf(outfile, "\tjmp     %s\n", label1);
		fprintf(outfile, "%s:\n", poplbl(1));
		exitloop();
		free(label1);
		exitscope(outfile);
		break;
	case WHILE:
		enterscope();
		pushlbl(newlabel(), newlabel());
		fprintf(outfile, "%s:\n", poplbl(0));
		writeexpr(stmt->val.whilestmt->cond, outfile);
		fprintf(outfile, "\tcmp     $0,%%rax\n");
		fprintf(outfile, "\tje      %s\n", poplbl(1));
		writestmt(stmt->val.whilestmt->stmt, outfile);
		fprintf(outfile, "\tjmp     %s\n", poplbl(0));
		fprintf(outfile, "%s:\n", poplbl(1));
		exitloop();
		exitscope(outfile);
		break;
	case DO:
		enterscope();
		label1 = newlabel();
		pushlbl(newlabel(), newlabel());
		fprintf(outfile, "%s:\n", label1);
		writestmt(stmt->val.whilestmt->stmt, outfile);
		fprintf(outfile, "%s:\n", poplbl(0));
		writeexpr(stmt->val.whilestmt->cond, outfile);
		fprintf(outfile, "\tcmp     $0,%%rax\n");
		fprintf(outfile, "\tje      %s\n", poplbl(1));
		fprintf(outfile, "\tjmp     %s\n", label1);
		fprintf(outfile, "%s:\n", poplbl(1));
		exitloop();
		free(label1);
		exitscope(outfile);
		break;
	case BREAK:
		fprintf(outfile, "\tjmp     %s\n", poplbl(1));
		break;
	case CONTINUE:
		fprintf(outfile, "\tjmp     %s\n", poplbl(0));
		break;
	case RET:
		writeexpr(stmt->val.expr, outfile);	
		fprintf(outfile, "\tmov     %%rbp,%%rsp\n");
		fprintf(outfile, "\tpop     %%rbp\n");
		fprintf(outfile, "\tret\n");
		break;
	case DECL:
		if (stmt->val.varstmt->init == NULL)
			fprintf(outfile, "\tmov     $0,%%rax\n");
		else
			writeexpr(stmt->val.varstmt->init, outfile);
		fprintf(outfile, "\tpush    %%rax\n");
		pushvar(stmt->val.varstmt->name);
		break;
	}
}

void clearstmt(struct statement *stmt)
{
	switch (stmt->act) {
	case EXPR:
	case BREAK:
	case CONTINUE:
	case RET:
		if (stmt->val.expr)
			clearexpr(stmt->val.expr);
		break;
	case SCOPE:
		clearbdy(stmt->val.bdy);
		break;
	case COND:
		clearexpr(stmt->val.condstmt->cond);
		if (stmt->val.condstmt->ifstmt)
			clearstmt(stmt->val.condstmt->ifstmt);
		if (stmt->val.condstmt->elsestmt)
			clearstmt(stmt->val.condstmt->elsestmt);
		free(stmt->val.condstmt);
		break;
	case FOR:
		if (stmt->val.forstmt->inittype == EXPR && stmt->val.forstmt->init.expr)
			clearexpr(stmt->val.forstmt->init.expr);
		else
			clearstmt(stmt->val.forstmt->init.stmt);
		if (stmt->val.forstmt->cond)
			clearexpr(stmt->val.forstmt->cond);
		if (stmt->val.forstmt->iter)
			clearexpr(stmt->val.forstmt->iter);
		if (stmt->val.forstmt->stmt)
			clearstmt(stmt->val.forstmt->stmt);
		free(stmt->val.forstmt);
		break;
	case WHILE:
	case DO:
		clearexpr(stmt->val.whilestmt->cond);
		if (stmt->val.whilestmt->stmt)
			clearstmt(stmt->val.whilestmt->stmt);
		free(stmt->val.whilestmt);
		break;
	case DECL:
		free(stmt->val.varstmt->name);
		if (stmt->val.varstmt->init)
			clearexpr(stmt->val.varstmt->init);
		free(stmt->val.varstmt);
		break;
	}
	free(stmt);
}

