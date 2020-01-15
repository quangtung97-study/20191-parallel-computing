.PHONY: all test

all:
	mpicc main.c -o main && mpirun -np 2 --oversubscribe main < input

test:
	python heat_map.py
