#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_SPACE 2048
#define BORDER_VALUE 0.0

float heat_map1[MAX_SPACE * MAX_SPACE];
float heat_map2[MAX_SPACE * MAX_SPACE];
float *A = heat_map1;
float *B = heat_map2;
size_t space_size;

float get(float *map, int i, int j) {
    if (j < 0 || j > space_size - 1)
        return BORDER_VALUE;
    if (i < 0 || i > space_size - 1)
        return BORDER_VALUE;

    return map[i * space_size + j];
}

void set(float *map, size_t i, size_t j, float value) {
    map[i * space_size + j] = value;
}

void init() {
    size_t region_count;

    FILE *file = fopen("simple_input", "r");
    fscanf(file, "%ld", &space_size);
    fscanf(file, "%ld", &region_count);

    size_t region_size = space_size / region_count;
    for (size_t i = 0; i < region_count; i++) {
        for (size_t j = 0; j < region_count; j++) {
            float value;
            fscanf(file, "%f", &value);
            for (size_t u = 0; u < region_size; u++) {
                for (size_t v = 0; v < region_size; v++) {
                    set(A, i * region_size + u, j * region_size + v, value);
                }
            }
        }
    }

    fclose(file);
}

void simulate() {
    for (int i = 0; i < space_size; i++)
        for (int j = 0; j < space_size; j++) {
            float sum = get(A, i, j - 1) + get(A, i - 1, j) -
                        4.0 * get(A, i, j) + get(A, i + 1, j) +
                        get(A, i, j + 1);
            sum *= 0.01;
            sum += get(A, i, j);
            B[i * space_size + j] = sum;
        }
    float *tmp = B;
    B = A;
    A = tmp;
}

void write_data(const char *filename) {
    FILE *file = fopen(filename, "w");

    fprintf(file, "%ld\n", space_size);

    for (size_t i = 0; i < space_size * space_size; i++) {
        fprintf(file, "%f\n", A[i]);
    }

    fclose(file);
}

int main() {
    srand(time(NULL));
    init();
    write_data("heat-data/0");
    for (size_t i = 0; i < 200; i++) {
        simulate();
    }
    write_data("heat-data/1");
    for (size_t i = 0; i < 1000; i++) {
        simulate();
    }
    write_data("heat-data/2");
    for (size_t i = 0; i < 2000; i++) {
        simulate();
    }
    write_data("heat-data/3");
    return 0;
}
