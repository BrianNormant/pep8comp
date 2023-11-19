CC=gcc
CFLAGS= -g -Werror -Wall -Wextra -Wreturn-type -pedantic -lpcre2-8

OBJECTS=bin/parser.o bin/main.o bin/parser_utils.o


bin/main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o bin/main.o

bin/parser.o: parser.c parser.h
	$(CC) $(CFLAGS) -c parser.c -o bin/parser.o

bin/parser_utils.o: parser_utils.c parser_utils.h
	$(CC) $(CFLAGS) -c parser_utils.c -o bin/parser_utils.o

build: $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) -fsanitize=undefined,address -o bin/main

release: $(OBJECTS)
	$(CC) $(OBJECTS) -lpcre2-8 -O2 -o bin/pep8comp

run: build
	./bin/main test.pep

clean:
	rm bin/*
