all: main.c image.c
	gcc image.c main.c -g -lm
run:
	./a.out
valgrind:
	valgrind --leak-check=full -v ./a.out
