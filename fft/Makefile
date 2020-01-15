.PHONY: all test

all:
	gcc simple.c -lm -o simple && ./simple

test:
	gcc generate.c -o generate && ./generate
	mpicc main.c -lm -o main && mpirun -np 2 --oversubscribe main < input
