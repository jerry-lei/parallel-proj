all: main.c image.c
	gcc image.c main.c -g -lm
	./a.out
run:
	./a.out
valgrind:
	valgrind --leak-check=full -v ./a.out
