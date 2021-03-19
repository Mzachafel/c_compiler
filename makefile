compiler: lexer.l parser.y ast.c main.c
	bison -d parser.y
	flex lexer.l
	gcc -g -o mzc ast.c parser.tab.c lex.yy.c main.c -lfl

test: prog.c
	./mzc prog.c
	gcc prog.s -o prog
	./prog
