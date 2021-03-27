compiler:
	bison -d parser.y
	flex lexer.l
	gcc -g -o mzc util.c expression.c statement.c function.c parser.tab.c lex.yy.c main.c -lfl

test:
	./mzc prog.c
	gcc prog.s -o prog
	./prog
