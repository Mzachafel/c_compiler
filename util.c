#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

struct declaration {
	char *name;
	int type;
	int value;
	int scope;
};

struct label {
	char *brk;
	char *cont;
};

#define DEFMAXDECL 8
#define DEFMAXLBL 8
#define DEFMAXFUNC 8
struct variable_mapping {
	/* known variables and function names */
	int maxdecl;
	struct declaration **decl_list;
	int curdecl;
	int stack_index;
	/* scope support for braces and loops */
	int curscope;
	/* iteration labels for break and continue statements */
	int maxlbl;
	struct label **lbl_list;
	int curlbl;
	/* defined functions' names */
	int maxfunc;
	char **func_list;
	int curfunc;
} *varmap;

void creatvarmap(void)
{
	varmap = (struct variable_mapping *) malloc(sizeof(struct variable_mapping));
	varmap->maxdecl = DEFMAXDECL;
	varmap->decl_list = (struct declaration **) calloc(varmap->maxdecl, sizeof(struct declaration *));
	varmap->curdecl = 0;
	varmap->stack_index = -8;
	varmap->curscope = 0;
	varmap->maxlbl = DEFMAXLBL;
	varmap->lbl_list = (struct label **) calloc(varmap->maxlbl, sizeof(struct label *));
	varmap->curlbl = 0;
	varmap->maxfunc = DEFMAXFUNC;
	varmap->func_list = (char **) calloc(varmap->maxfunc, sizeof(char *));
	varmap->curfunc = 0;
}

void pushvar(char *name, int value)
{
	for (int i = varmap->curdecl-1; i >= 0; i--)
		if (!strcmp(varmap->decl_list[i]->name, name) && 
		    varmap->decl_list[i]->type == 0 /* var */ &&
		    varmap->decl_list[i]->scope == varmap->curscope) {
			fprintf(stderr, "Declared variable %s few times\n", name);
			exit(1);
		}
	if (varmap->curdecl == varmap->maxdecl) {
		varmap->maxdecl *= 2;
		varmap->decl_list = (struct declaration **) 
			            realloc(varmap->decl_list, varmap->maxdecl * sizeof(struct declaration *));
	}
	varmap->decl_list[varmap->curdecl] = (struct declaration *) malloc(sizeof(struct declaration));
	varmap->decl_list[varmap->curdecl]->name = strdup(name);
	varmap->decl_list[varmap->curdecl]->type = 0 /* var */;
	varmap->decl_list[varmap->curdecl]->value = value ? value : varmap->stack_index;
	varmap->decl_list[varmap->curdecl]->scope = varmap->curscope;
	varmap->curdecl++;
	varmap->stack_index -= 8;
}

int popvar(char *name)
{
	int i;
	for (i = varmap->curdecl-1; i >= 0; i--)
		if (!strcmp(varmap->decl_list[i]->name, name) && varmap->decl_list[i]->type == 0 /* var */)
			break;
	if (i < 0) {
		fprintf(stderr, "Variable %s is not declared\n", name);
		exit(1);
	} else
		return varmap->decl_list[i]->value;
}

void pushfunc(char *name, int value)
{
	for (int i = varmap->curdecl-1; i >= 0; i--) {
		if (!strcmp(varmap->decl_list[i]->name, name) && 
		    varmap->decl_list[i]->type == 1 /* func */ &&
		    varmap->decl_list[i]->scope == varmap->curscope)
			return;
		if (varmap->decl_list[i]->scope != varmap->curscope)
			break;
	}
	if (varmap->curdecl == varmap->maxdecl) {
		varmap->maxdecl *= 2;
		varmap->decl_list = (struct declaration **) 
			            realloc(varmap->decl_list, varmap->maxdecl * sizeof(struct declaration *));
	}
	varmap->decl_list[varmap->curdecl] = (struct declaration *) malloc(sizeof(struct declaration));
	varmap->decl_list[varmap->curdecl]->name = strdup(name);
	varmap->decl_list[varmap->curdecl]->type = 1 /* func */;
	varmap->decl_list[varmap->curdecl]->value = value;
	varmap->decl_list[varmap->curdecl]->scope = varmap->curscope;
	varmap->curdecl++;
}

void popfunc(char *name, int value)
{
	int i;
	for (i = varmap->curdecl-1; i >= 0; i--)
		if (!strcmp(varmap->decl_list[i]->name, name))
			if (varmap->decl_list[i]->type == 1 /* func */)
				break;
			else {
				fprintf(stderr, "Called object %s is not a function\n", name);
				exit(1);
			}
	if (i < 0) {
		fprintf(stderr, "Function %s is not declared\n", name);
		exit(1);
	}
	if (varmap->decl_list[i]->value != value) {
		fprintf(stderr, "Function %s has different amount of arguments\n", name);
		exit(1);
	}
}

void enterscope(void)
{
	varmap->curscope++;
}

void exitscope(FILE *outfile)
{
	int stack_ret = 0;
	for (int i = varmap->curdecl - 1; i >= 0; i--) {
		if (varmap->decl_list[i]->scope != varmap->curscope)
			break;
		else {
			free(varmap->decl_list[i]->name);
			free(varmap->decl_list[i]);
			varmap->curdecl--;
			stack_ret += 8;
		}
	}
	varmap->curscope--;
	if (stack_ret) {
		fprintf(outfile, "\tadd     $%d,%%rsp\n", stack_ret);
		varmap->stack_index += stack_ret;
	}
}

void pushlbl(char *brk, char *cont)
{
	if (varmap->curlbl == varmap->maxlbl) {
		varmap->maxlbl *= 2;
		varmap->lbl_list = (struct label **) 
			            realloc(varmap->lbl_list, varmap->maxlbl * sizeof(struct label *));
	}
	varmap->lbl_list[varmap->curlbl] = (struct label *) malloc(sizeof(struct label));
	varmap->lbl_list[varmap->curlbl]->brk = strdup(brk);
	varmap->lbl_list[varmap->curlbl]->cont = strdup(cont);
	varmap->curlbl++;
}

char *poplbl(int act)
{
	if (varmap->curlbl == 0) {
		fprintf(stderr, "Break statement not within loop\n");
		exit(1);
	}
	if (act)
		return varmap->lbl_list[varmap->curlbl-1]->brk;
	else
		return varmap->lbl_list[varmap->curlbl-1]->cont;
}

int checkfuncname(char *name)
{
	for (int i = 0; i<varmap->curfunc; i++)
		if (!strcmp(varmap->func_list[i], name))
			return 0;
	if (varmap->curfunc == varmap->maxfunc) {
		varmap->maxfunc *= 2;
		varmap->func_list = (char **) realloc(varmap->func_list, varmap->maxfunc * sizeof(char *));
	}
	varmap->func_list[varmap->curfunc++] = strdup(name);
	return 1;
}

void exitloop(void) {
	varmap->curlbl--;
	free(varmap->lbl_list[varmap->curlbl]->brk);
	free(varmap->lbl_list[varmap->curlbl]->cont);
	free(varmap->lbl_list[varmap->curlbl]);
}

void clearvarmap(void)
{
	for (int i=0; i<varmap->curdecl; i++) {
		free(varmap->decl_list[i]->name);
		free(varmap->decl_list[i]);
	}
	free(varmap->decl_list);
	free(varmap->lbl_list);
	free(varmap);
}

