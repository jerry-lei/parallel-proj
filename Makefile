all: main.c image.c hash.c hsv.c score.c
	mpicc hash.c image.c main.c hsv.c score.c -g -lm -pthread
fast: main.c image.c hash.c hsv.c score.c
	mpicc -O3 hash.c image.c main.c hsv.c score.c -g -lm -pthread
run:
	mpirun -n 4 ./a.out
valgrind:
	mpirun -n 16 valgrind --leak-check=full -v ./a.out
valgrind2:
	mpirun -n 16 valgrind --leak-check=full --show-leak-kinds=all -v ./a.out
valgrind3:
	mpirun -n 16 valgrind --leak-check=full --show-leak-kinds=all ./a.out
