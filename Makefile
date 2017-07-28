CC = gcc
CFLAGS=-I.

all: bin/snake

bin/snake: bin/snake.o bin/main.o
	gcc bin/snake.o bin/main.o -o bin/snake -pthread -lrt

bin/main.o: src/main.c bin/snake.o
	gcc -c src/main.c -o bin/main.o

bin/snake.o: src/snake.c src/snakeTypes.h src/snakeConstants.h
	gcc -c src/snake.c -o bin/snake.o -pthread -lrt

clean:
	rm bin/*.o bin/snake
