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

struct variable {
	char *name;
	int offset;
};

#define DEFMAXVAR 8
struct variable_mapping {
	int maxvar;
	struct variable **var_list;
	int curvar;
	int stack_index;
} *varmap;

void creatvarmap(void)
{
	varmap = (struct variable_mapping *) malloc(sizeof(struct variable_mapping));
	varmap->maxvar = DEFMAXVAR;
	varmap->var_list = (struct variable **) calloc(varmap->maxvar, sizeof(struct variable *));
	varmap->curvar = 0;
	varmap->stack_index = -8;
}

void pushvar(char *name)
{
	for (int i = varmap->curvar-1; i >= 0; i--)
		if (!strcmp(varmap->var_list[i]->name, name)) {
			printf("Declared variable %s few times\n", name);
			return;
		}
	if (varmap->curvar == varmap->maxvar) {
		varmap->maxvar *= 2;
		varmap->var_list = (struct variable **) 
			            realloc(varmap->var_list, varmap->maxvar * sizeof(struct variable *));
	}
	varmap->var_list[varmap->curvar] = (struct variable *) malloc(sizeof(struct variable));
	varmap->var_list[varmap->curvar]->name = strdup(name);
	varmap->var_list[varmap->curvar]->offset = varmap->stack_index;
	varmap->curvar++;
	varmap->stack_index -= 8;
}

int popvar(char *name)
{
	int i;
	for (i = varmap->curvar-1; i >= 0; i--)
		if (!strcmp(varmap->var_list[i]->name, name))
			break;
	if (i < 0) {
		printf("Variable %s is not declared\n", name);
		return 0;
	} else
		return varmap->var_list[i]->offset;
}

void clearvarmap(void)
{
	for (int i=0; i<varmap->curvar; i++) {
		free(varmap->var_list[i]->name);
		free(varmap->var_list[i]);
	}
	free(varmap->var_list);
	free(varmap);
}

