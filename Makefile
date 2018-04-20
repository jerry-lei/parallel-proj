all: main.c image.c hash.c hsv.c score.c
	gcc hash.c image.c main.c hsv.c score.c -g -lm -pthread
fast: main.c image.c hash.c hsv.c score.c
	gcc -O3 hash.c image.c main.c hsv.c score.c -g -lm -pthread
run:
	./a.out
valgrind:
	valgrind --leak-check=full -v ./a.out
valgrind2:
	valgrind --leak-check=full --show-leak-kinds=all -v ./a.out
valgrind3:
	valgrind --leak-check=full --show-leak-kinds=all ./a.out
