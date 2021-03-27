#include "ast.h"

union leftvalue {
	struct expression *expr;
	char *name;
	int num;
};

struct expression {
	union leftvalue lval;
	int act;
	struct expression *rexpr;
};

struct expression *creatbinexpr(struct expression *lval, int act, struct expression *rexpr)
{
	struct expression *expr = (struct expression *) malloc(sizeof(struct expression));
	expr->lval.expr = lval;
	expr->act = act;
	expr->rexpr = rexpr;

	return expr;
}

struct expression *creatunexpr(struct expression *lval, int act)
{
	struct expression *expr = (struct expression *) malloc(sizeof(struct expression));
	expr->lval.expr = lval;
	expr->act = act;
	expr->rexpr = NULL;

	return expr;
}

struct expression *creatvarexpr(char *name, int act, struct expression *rexpr)
{
	struct expression *expr = (struct expression *) malloc(sizeof(struct expression));
	expr->lval.name = strdup(name);
	expr->act = act;
	expr->rexpr = rexpr;

	return expr;
}

struct expression *creatconstexpr(int num, int act)
{
	struct expression *expr = (struct expression *) malloc(sizeof(struct expression));
	expr->lval.num = num;
	expr->act = act;
	expr->rexpr = NULL;

	return expr;
}

void writeexpr(struct expression *expr, FILE *outfile)
{
	char *label1, *label2;
	int offset;
	if (expr->act == CONSTANT)
		fprintf(outfile, "\tmov     $%d,%%rax\n", expr->lval.num);
	else if (expr->rexpr == NULL) /* unary operator */
		switch (expr->act) {
		case NEG: /* negotiation:
                	     x -> -x */
			writeexpr(expr->lval.expr, outfile);
			fprintf(outfile, "\tneg     %%rax\n");
			break;
		case LNEG: /* logical negotiation:
	        	     0 -> 1, anything else -> 0 */
			writeexpr(expr->lval.expr, outfile);
			fprintf(outfile, "\tcmp     $0,%%rax\n");
			fprintf(outfile, "\tsete    %%al\n");
			break;
		case NOT: /* bitwise complement:
		             0110 -> 1001 */
			writeexpr(expr->lval.expr, outfile);
			fprintf(outfile, "\tnot     %%rax\n");
			break;
		case PREFINC:
			offset = popvar(expr->lval.name);
			fprintf(outfile, "\tmov     %d(%%rbp),%%rax\n", offset);
			fprintf(outfile, "\tinc     %%rax\n");
			fprintf(outfile, "\tmov     %%rax,%d(%%rbp)\n", offset);
			break;
		case PREFDEC:
			offset = popvar(expr->lval.name);
			fprintf(outfile, "\tmov     %d(%%rbp),%%rax\n", offset);
			fprintf(outfile, "\tdec     %%rax\n");
			fprintf(outfile, "\tmov     %%rax,%d(%%rbp)\n", offset);
			break;
		case IDENTIFIER: /* get value of variable */
			offset = popvar(expr->lval.name);
			fprintf(outfile, "\tmov     %d(%%rbp),%%rax\n", offset);
			break;
		case POSTINC:
			offset = popvar(expr->lval.name);
			fprintf(outfile, "\tmov     %d(%%rbp),%%rax\n", offset);
			fprintf(outfile, "\tmov     %%rax,%%rcx\n");
			fprintf(outfile, "\tinc     %%rcx\n");
			fprintf(outfile, "\tmov     %%rcx,%d(%%rbp)\n", offset);
			break;
		case POSTDEC:
			offset = popvar(expr->lval.name);
			fprintf(outfile, "\tmov     %d(%%rbp),%%rax\n", offset);
			fprintf(outfile, "\tmov     %%rax,%%rcx\n");
			fprintf(outfile, "\tdec     %%rcx\n");
			fprintf(outfile, "\tmov     %%rcx,%d(%%rbp)\n", offset);
			break;
		}
	else /* binary operator */
		switch(expr->act) {
		/* assign value to variable */
		case A:
			offset = popvar(expr->lval.name);
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tmov     %%rax,%d(%%rbp)\n", offset);
			break;
		case BORA:
			offset = popvar(expr->lval.name);
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tor      %%rax,%d(%%rbp)\n", offset);
			break;
		case BXORA:
			offset = popvar(expr->lval.name);
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\txor     %%rax,%d(%%rbp)\n", offset);
			break;
		case BANDA:
			offset = popvar(expr->lval.name);
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tand     %%rax,%d(%%rbp)\n", offset);
			break;
		case SHLA:
			offset = popvar(expr->lval.name);
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tmov     %%al,%%cl\n");
			fprintf(outfile, "\tshl     %%cl,%d(%%rbp)\n", offset);
			break;
		case SHRA:
			offset = popvar(expr->lval.name);
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tmov     %%al,%%cl\n");
			fprintf(outfile, "\tshr     %%cl,%d(%%rbp)\n", offset);
			break;
		case ADDA:
			offset = popvar(expr->lval.name);
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tadd     %%rax,%d(%%rbp)\n", offset);
			break;
		case SUBA:
			offset = popvar(expr->lval.name);
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tsub     %%rax,%d(%%rbp)\n", offset);
			break;
		case MULA:
			offset = popvar(expr->lval.name);
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tmul     %d(%%rbp)\n", offset);
			fprintf(outfile, "\tmov     %%rax,%d(%%rbp)\n", offset);
			break;
		case DIVA:
			offset = popvar(expr->lval.name);
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tmov     %%rax,%%rcx\n");
			fprintf(outfile, "\tmov     %d(%%rbp),%%rax\n", offset);
			fprintf(outfile, "\tmov     $0,%%rdx\n");
			fprintf(outfile, "\tdiv     %%rcx\n");
			fprintf(outfile, "\tmov     %%rax,%d(%%rbp)\n", offset);
			break;
		case MODA:
			offset = popvar(expr->lval.name);
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tmov     %%rax,%%rcx\n");
			fprintf(outfile, "\tmov     %d(%%rbp),%%rax\n", offset);
			fprintf(outfile, "\tmov     $0,%%rdx\n");
			fprintf(outfile, "\tdiv     %%rcx\n");
			fprintf(outfile, "\tmov     %%rdx,%d(%%rbp)\n", offset);
			break;
		/* logical or arithmetic operations */
		case COMMA:
			writeexpr(expr->lval.expr, outfile);
			writeexpr(expr->rexpr, outfile);
			break;
		case OR:
			writeexpr(expr->lval.expr, outfile);
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
			writeexpr(expr->lval.expr, outfile);
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
		case BOR:
			writeexpr(expr->lval.expr, outfile);
			fprintf(outfile, "\tpush    %%rax\n");
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tpop     %%rcx\n");
			fprintf(outfile, "\tor      %%rcx,%%rax\n");
			break;
		case BXOR:
			writeexpr(expr->lval.expr, outfile);
			fprintf(outfile, "\tpush    %%rax\n");
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tpop     %%rcx\n");
			fprintf(outfile, "\txor     %%rcx,%%rax\n");
			break;
		case BAND:
			writeexpr(expr->lval.expr, outfile);
			fprintf(outfile, "\tpush    %%rax\n");
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tpop     %%rcx\n");
			fprintf(outfile, "\tand     %%rcx,%%rax\n");
			break;
		case E:
			writeexpr(expr->lval.expr, outfile);
			fprintf(outfile, "\tpush    %%rax\n");
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tpop     %%rcx\n");
			fprintf(outfile, "\tcmp     %%rcx,%%rax\n");
			fprintf(outfile, "\tsete    %%al\n");
			break;
		case NE:
			writeexpr(expr->lval.expr, outfile);
			fprintf(outfile, "\tpush    %%rax\n");
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tpop     %%rcx\n");
			fprintf(outfile, "\tcmp     %%rcx,%%rax\n");
			fprintf(outfile, "\tsetne    %%al\n");
			break;
		case L:
			writeexpr(expr->lval.expr, outfile);
			fprintf(outfile, "\tpush    %%rax\n");
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tpop     %%rcx\n");
			fprintf(outfile, "\tcmp     %%rcx,%%rax\n");
			fprintf(outfile, "\tsetg    %%al\n");
			break;
		case LE:
			writeexpr(expr->lval.expr, outfile);
			fprintf(outfile, "\tpush    %%rax\n");
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tpop     %%rcx\n");
			fprintf(outfile, "\tcmp     %%rcx,%%rax\n");
			fprintf(outfile, "\tsetge   %%al\n");
			break;
		case G:
			writeexpr(expr->lval.expr, outfile);
			fprintf(outfile, "\tpush    %%rax\n");
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tpop     %%rcx\n");
			fprintf(outfile, "\tcmp     %%rcx,%%rax\n");
			fprintf(outfile, "\tsetl    %%al\n");
			break;
		case GE:
			writeexpr(expr->lval.expr, outfile);
			fprintf(outfile, "\tpush    %%rax\n");
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tpop     %%rcx\n");
			fprintf(outfile, "\tcmp     %%rcx,%%rax\n");
			fprintf(outfile, "\tsetle   %%al\n");
			break;
		case SHL:
			writeexpr(expr->lval.expr, outfile);
			fprintf(outfile, "\tpush    %%rax\n");
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tmov     %%al,%%cl\n");
			fprintf(outfile, "\tpop     %%rax\n");
			fprintf(outfile, "\tshl     %%cl,%%rax\n");
			break;
		case SHR:
			writeexpr(expr->lval.expr, outfile);
			fprintf(outfile, "\tpush    %%rax\n");
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tmov     %%al,%%cl\n");
			fprintf(outfile, "\tpop     %%rax\n");
			fprintf(outfile, "\tshr     %%cl,%%rax\n");
			break;
		case ADD:
			writeexpr(expr->lval.expr, outfile);
			fprintf(outfile, "\tpush    %%rax\n");
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tpop     %%rcx\n");
			fprintf(outfile, "\tadd     %%rcx,%%rax\n");
			break;
		case SUB:
			writeexpr(expr->lval.expr, outfile);
			fprintf(outfile, "\tpush    %%rax\n");
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tpop     %%rcx\n");
			fprintf(outfile, "\tsub     %%rax,%%rcx\n");
			fprintf(outfile, "\tmov     %%rcx,%%rax\n");
			break;
		case MUL:
			writeexpr(expr->lval.expr, outfile);
			fprintf(outfile, "\tpush    %%rax\n");
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tpop     %%rcx\n");
			fprintf(outfile, "\tmul     %%rcx\n");
			break;
		case DIV:
			writeexpr(expr->lval.expr, outfile);
			fprintf(outfile, "\tpush    %%rax\n");
			writeexpr(expr->rexpr, outfile);
			fprintf(outfile, "\tmov     %%rax,%%rcx\n");
			fprintf(outfile, "\tpop     %%rax\n");
			fprintf(outfile, "\tmov     $0,%%rdx\n");
			fprintf(outfile, "\tdiv     %%rcx\n");
			break;
		case MOD:
			writeexpr(expr->lval.expr, outfile);
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

void clearexpr(struct expression *expr)
{
	if (expr->act == IDENTIFIER || expr->act >= PREFINC && expr->act <= POSTDEC || 
	    expr->act >= A && expr->act <= BANDA)
		free(expr->lval.name);
	else if (expr->act != CONSTANT)
		clearexpr(expr->lval.expr);
	if (expr->rexpr)
		clearexpr(expr->rexpr);
	free(expr);
}
