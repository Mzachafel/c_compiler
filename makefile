parser: lexer.l parser.y ast.c main.c
	bison -d parser.y
	flex lexer.l
	gcc -o mzc ast.c parser.tab.c lex.yy.c main.c -lfl
