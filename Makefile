OBJECTS=bin/parser.o bin/main.o
CFLAGS= -g -Werror -Wall -Wextra -Wreturn-type -pedantic -lpcre2-8
bin/parser.o: parser.c
	gcc $(CFLAGS) -c parser.c -o bin/parser.o

bin/main.o: main.c
	gcc $(CFLAGS) -c main.c -o bin/main.o


build: $(OBJECTS)
	gcc $(OBJECTS) $(CFLAGS) -fsanitize=undefined,address -o bin/main

run: build
	./bin/main test.pep

clean:
	rm bin/*
