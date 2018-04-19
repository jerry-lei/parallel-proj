all: main.c image.c hash.c hsv.c score.c
	gcc hash.c image.c main.c hsv.c score.c -g -lm -pthread -O3
run:
	./a.out
valgrind:
	valgrind --leak-check=full --show-leak-kinds=all ./a.out
