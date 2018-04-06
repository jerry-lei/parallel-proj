all: main.c image.c
	gcc *.c
run:
	./a.out
leak:
	valgrind --leak-check=full -v ./a.out
