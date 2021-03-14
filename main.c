#include <stdio.h>
#include <string.h>
#include "parser.tab.h"

extern FILE *yyin;
FILE *outfile;

int main(int argc, char **argv)
{
	if (argc < 2) {
		fputs("error: no file to compile\n", stderr);
		return 1;
	} else if (!(yyin = fopen(argv[1], "r"))) {
		perror(argv[1]);
		return 1;
	}

	argv[1][strlen(argv[1])-1] = 's';
	if (!(outfile = fopen(argv[1], "w"))) {
		perror(argv[1]);
		return 1;
	}
	
	yyparse();

	fclose(outfile);
	return 0;
}
