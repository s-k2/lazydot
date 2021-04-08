lazydot: parser.tab.c lex.yy.c filter.cpp filter.h
	g++ -Wall -Werror -std=c++11 -o lazydot parser.tab.c lex.yy.c filter.cpp -lfl 

parser.tab.c parser.tab.h: parser.y filter.h
	bison -d parser.y 

lex.yy.c: lex.l parser.tab.h
	flex lex.l

clean:
	rm -f parser.tab.c parser.tab.h lex.yy.c lazydot

example: lazydot
	./lazydot example.lazydot
