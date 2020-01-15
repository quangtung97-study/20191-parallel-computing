#include <math.h>
#include <stdio.h>
#include <complex.h>

typedef double complex complex_t;

complex_t complex_exp(double angle) {
    return cos(angle) + sin(angle) * I;
}

void simple_fft(const complex_t *input, complex_t *output, size_t N) {
    for (size_t k = 0; k < N; k++) {
        complex_t sum = 0.0;
        for (size_t n = 0; n < N; n++) {
            sum += input[n] * complex_exp(-2 * M_PI * n * k / N);
        }
        output[k] = sum;
    }
}

size_t inverse_bits(size_t value, size_t logN) {
    size_t result = 0;
    for (size_t i = 0; i < logN; i++) {
        if (value & (1 << i)) {
            result |= 1 << (logN - i - 1);
        }
    }
    return result;
}

void print(const complex_t *a, size_t N) {
    for (size_t n = 0; n < N; n++)
        printf("[%lf + %lfi] ", creal(a[n]), cimag(a[n]));
    printf("\n");
}

void inverse_bits_array(size_t *array, size_t logN) {
    const size_t N = 1 << logN;
    for (size_t i = 0; i < N; i++) {
        array[i] = inverse_bits(i, logN);
    }
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

static size_t bits_array[10000];

void fft(complex_t *array, size_t logN) {
    exchange_array(array, logN);

    const size_t N = 1 << logN;
    printf("%ld\n", N);

    for (size_t step = 1; step < N; step *= 2) {
        for (size_t block = 0; block < N; block += step * 2) {
            for (size_t i = 0; i < step; i++) {
                complex_t e = complex_exp(-2.0 * M_PI * i / (2 * step));

                complex_t a = array[block + i];
                complex_t b = array[block + i + step];

                array[block + i] = a + e * b;
                array[block + i + step] = a - e * b;
            }
        }
    }
}

int main() {
    complex_t input[] = {0, 1, 2, 3, 4, 5, 6, 7};
    complex_t output[8];

    simple_fft(input, output, 8);
    fft(input, 3);

    print(input, 8);
    print(output, 8);
    return 0;
}
