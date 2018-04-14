all: main.c image.c hash.c
	gcc hash.c image.c main.c -g -lm -pthread
run:
	./a.out
valgrind:
	valgrind --leak-check=full -v ./a.out
