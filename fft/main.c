#include <complex.h>
#include <math.h>
#include <mpi.h>
#include <stdio.h>

#define MAX_LOGN 22
#define MAX_N (1 << MAX_LOGN)
typedef double complex complex_t;

size_t logN;
complex_t A[MAX_N];
complex_t B[MAX_N];

size_t log_int(size_t N) {
    size_t log = 0;
    while ((1 << log) <= N) {
        log++;
    }
    return log - 1;
}

complex_t complex_exp(double angle) { return cos(angle) + sin(angle) * I; }

size_t inverse_bits(size_t value, size_t logN) {
    size_t result = 0;
    for (size_t i = 0; i < logN; i++) {
        if (value & (1 << i)) {
            result |= 1 << (logN - i - 1);
        }
    }
    return result;
}

void exchange_array(complex_t *array, size_t logN) {
    const size_t N = 1 << logN;
    for (size_t i = 0; i < N / 2; i++) {
        size_t inverted_i = inverse_bits(i, logN);
        complex_t tmp = array[i];
        array[i] = array[inverted_i];
        array[inverted_i] = tmp;
    }
}

void read_from_stdin() {
    scanf("%ld", &logN);
    size_t N = 1 << logN;
    for (size_t i = 0; i < N; i++) {
        double value;
        scanf("%lf", &value);
        A[i] = value;
    }
    exchange_array(A, logN);
}

complex_t exp_array[MAX_N];

void fft(complex_t *array, size_t logN, size_t step_begin, size_t step_end) {
    const size_t N = 1 << logN;

    for (size_t step = step_begin; step < step_end; step *= 2) {
        const complex_t delta_exp = complex_exp(-M_PI / step);
        complex_t exp = 1.0;
        for (size_t i = 0; i < step; i++) {
            exp_array[i] = exp;
            exp *= delta_exp;
        }

        for (size_t block = 0; block < N; block += step * 2) {
            for (size_t i = 0; i < step; i++) {
                const complex_t e = exp_array[i];

                const complex_t a = array[block + i];
                const complex_t b = array[block + i + step];

                array[block + i] = a + e * b;
                array[block + i + step] = a - e * b;
            }
        }
    }
}

void print(int rank, const complex_t *a, size_t N) {
    printf("%d: ", rank);
    for (size_t n = 0; n < N; n++)
        printf("[%lf + %lfi] ", creal(a[n]), cimag(a[n]));
    printf("\n");
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int __process_count;
    MPI_Comm_size(MPI_COMM_WORLD, &__process_count);

    size_t log_process_count = log_int(__process_count);
    size_t process_count = 1 << log_process_count;
    int color = rank < process_count ? 0 : 1;

    float total_trans_time = 0.0;
    float start, tmp_start;

    MPI_Comm comm;
    MPI_Comm_split(MPI_COMM_WORLD, color, rank, &comm);

    if (rank < process_count) {
        if (rank == 0)
            read_from_stdin();

        tmp_start = MPI_Wtime();
        start = tmp_start;
        MPI_Bcast(&logN, 1, MPI_UINT64_T, 0, comm);
        total_trans_time += MPI_Wtime() - tmp_start;

        size_t log_num = (logN - log_process_count);
        size_t num = 1 << log_num;

        tmp_start = MPI_Wtime();
        MPI_Scatter(A, num * 2, MPI_DOUBLE, B, num * 2, MPI_DOUBLE, 0,
                    comm);
        total_trans_time += MPI_Wtime() - tmp_start;

        fft(B, log_num, 1, num);

        tmp_start = MPI_Wtime();
        MPI_Gather(B, num * 2, MPI_DOUBLE, A, num * 2, MPI_DOUBLE, 0,
                   comm);
        total_trans_time += MPI_Wtime() - tmp_start;

        if (rank == 0) {
            fft(A, logN, num, 1 << logN);

            float end = MPI_Wtime();
            printf("Total time: %f\n", end - start);
            printf("Calculation time: %f\n", end - start - total_trans_time);
            printf("Transmission time: %f\n", total_trans_time);
            // print(0, A, 1 << logN);
        }
    }

    MPI_Finalize();
    return 0;
}
