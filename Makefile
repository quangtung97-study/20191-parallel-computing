.PHONY: all test

all:
	mpicc mpi_mfas.c -lm -o main && mpirun ./main < input2

test:
	mpicc mpi_fft.c -lm -o main && mpirun -np 4 --oversubscribe main < input
