#include <stdio.h>

int main() {
    FILE *file = fopen("input", "w");

    size_t logN = 22;
    size_t N = 1 << logN;
    fprintf(file, "%ld\n", logN);
    for (size_t i = 0; i < N; i++) {
        fprintf(file, "%ld ", i);
    }
    fprintf(file, "\n");

    fclose(file);
    return 0;
}
