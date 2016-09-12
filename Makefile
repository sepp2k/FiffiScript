FLAGS=-std=c++14 -ggdb -ldl -I src -isystem gen $(shell pkg-config --cflags --libs libffi)
WARN_FLAGS=-Wall -pedantic
CXX=g++ ${FLAGS} ${WARN_FLAGS}
CXX_NOWARN=g++ ${FLAGS}

.PHONY: all clean examples

all: fiffiscript examples

gen/parser.tab.cc gen/parser.tab.hh gen/stack.hh: src/parser.yy src/fiffiscript.hh src/util.hh
	mkdir -p gen
	bison -v --file-prefix=gen/parser src/parser.yy

parser.tab.o: gen/parser.tab.cc gen/parser.tab.hh gen/stack.hh src/fiffiscript.hh src/util.hh
	mkdir -p gen
	${CXX_NOWARN} -c gen/parser.tab.cc

gen/lex.yy.c: src/tokenizer.l
	flex -o gen/lex.yy.c src/tokenizer.l

lex.yy.o: gen/lex.yy.c gen/parser.tab.hh gen/stack.hh src/util.hh src/tokenizer.hh
	${CXX_NOWARN} -c gen/lex.yy.c

fiffiscript.o: src/fiffiscript.cc src/fiffiscript.hh src/util.hh
	${CXX} -c src/fiffiscript.cc

main.o: src/main.cc gen/parser.tab.hh gen/stack.hh src/util.hh src/tokenizer.hh
	${CXX} -c src/main.cc

fiffiscript: main.o fiffiscript.o lex.yy.o parser.tab.o
	${CXX} -o fiffiscript main.o fiffiscript.o parser.tab.o lex.yy.o

external_lib.so: examples/external_lib.c
	gcc -shared -o external_lib.so -fPIC examples/external_lib.c

examples: external_lib.so

clean:
	rm -rfv *.o *.so gen fiffiscript
