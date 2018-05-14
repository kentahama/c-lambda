parser: lambda-parser.tab.c lex.yy.c lambda.c
	gcc -o $@ $^ -lfl -ly

lex.yy.c: lambda-parser.l
	flex $^

lambda-parser.tab.c: lambda-parser.y
	bison -d $^

main: main.c lambda.c
	gcc -o $@ $^
