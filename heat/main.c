#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_SPACE 2048
#define BORDER_VALUE 0.0

float initial_heat_map[MAX_SPACE * MAX_SPACE];

float heat_map1[MAX_SPACE * MAX_SPACE];
float heat_map2[MAX_SPACE * MAX_SPACE];

float *A = heat_map1;
float *B = heat_map2;

float top_border[MAX_SPACE] = {BORDER_VALUE};
float bottom_border[MAX_SPACE] = {BORDER_VALUE};

float h;
size_t space_size, space_width, space_height;

void set_initial(size_t i, size_t j, float value) {
    initial_heat_map[i * space_size + j] = value;
}

void read_data() {
    size_t region_count;

    FILE *file = fopen("input", "r");
    fscanf(file, "%ld", &space_size);
    fscanf(file, "%ld", &region_count);

    size_t region_size = space_size / region_count;
    for (size_t i = 0; i < region_count; i++) {
        for (size_t j = 0; j < region_count; j++) {
            float value;
            fscanf(file, "%f", &value);
            for (size_t u = 0; u < region_size; u++) {
                for (size_t v = 0; v < region_size; v++) {
                    set_initial(i * region_size + u, j * region_size + v,
                                value);
                }
            }
        }
    }

    fclose(file);
}

void scatter_initial_map(int process_count, float *total_trans_time) {
    int *sendcounts = malloc(sizeof(int) * process_count);
    int *displs = malloc(sizeof(int) * process_count);

    for (int i = 0; i < process_count; i++) {
        sendcounts[i] = (i + 1) * space_size / process_count -
                        i * space_size / process_count;
        sendcounts[i] *= space_size;

        displs[i] = i * space_size / process_count;
        displs[i] *= space_size;
    }

    float before = MPI_Wtime();
    PMPI_Scatterv(initial_heat_map, sendcounts, displs, MPI_FLOAT, A,
                  MAX_SPACE * MAX_SPACE, MPI_FLOAT, 0, MPI_COMM_WORLD);
    *total_trans_time += MPI_Wtime() - before;

    free(displs);
    free(sendcounts);
}

void gather_initial_map(int process_count, float *total_trans_time) {
    int *recvcounts = malloc(sizeof(int) * process_count);
    int *displs = malloc(sizeof(int) * process_count);

    for (int i = 0; i < process_count; i++) {
        recvcounts[i] = (i + 1) * space_size / process_count -
                        i * space_size / process_count;
        recvcounts[i] *= space_size;

        displs[i] = i * space_size / process_count;
        displs[i] *= space_size;
    }

    float before = MPI_Wtime();
    MPI_Gatherv(A, space_width * space_height, MPI_FLOAT, initial_heat_map,
                recvcounts, displs, MPI_FLOAT, 0, MPI_COMM_WORLD);
    *total_trans_time += MPI_Wtime() - before;

    free(displs);
    free(recvcounts);
}

#define TAG_BORDER_TOP 121
#define TAG_BORDER_BOTTOM 122

void send_borders(int rank, int process_count, float *total_trans_time) {
    float before = MPI_Wtime();
    if (rank > 0) {
        MPI_Send(A, space_width, MPI_FLOAT, rank - 1, TAG_BORDER_BOTTOM,
                 MPI_COMM_WORLD);
    }

    if (rank < process_count - 1) {
        MPI_Send(A + space_width * (space_height - 1), space_width, MPI_FLOAT,
                 rank + 1, TAG_BORDER_TOP, MPI_COMM_WORLD);
    }
    *total_trans_time += MPI_Wtime() - before;
}

void recv_borders(int rank, int process_count, float *total_trans_time) {
    float before = MPI_Wtime();
    if (rank < process_count - 1) {
        MPI_Status status;
        MPI_Recv(bottom_border, space_width, MPI_FLOAT, rank + 1,
                 TAG_BORDER_BOTTOM, MPI_COMM_WORLD, &status);
    }

    if (rank > 0) {
        MPI_Status status;
        MPI_Recv(top_border, space_width, MPI_FLOAT, rank - 1, TAG_BORDER_TOP,
                 MPI_COMM_WORLD, &status);
    }
    *total_trans_time += MPI_Wtime() - before;
}

void calculate_space_width_height(int rank, int process_count) {
    space_width = space_size;
    space_height = (rank + 1) * space_size / process_count -
                   rank * space_size / process_count;
}

float get(float *map, int i, int j) {
    if (j < 0 || j > space_width - 1)
        return BORDER_VALUE;

    if (i < 0)
        return top_border[j];

    if (i > space_height - 1)
        return bottom_border[j];

    return map[i * space_width + j];
}

void simulate() {
    for (int i = 0; i < space_height; i++)
        for (int j = 0; j < space_width; j++) {
            float sum = get(A, i, j - 1) + get(A, i - 1, j) -
                        4.0 * get(A, i, j) + get(A, i + 1, j) +
                        get(A, i, j + 1);
            sum *= 0.000001 / (h * h);
            sum += get(A, i, j);
            B[i * space_width + j] = sum;
        }
    float *tmp = B;
    B = A;
    A = tmp;
}

void write_data(const char *filename) {
    FILE *file = fopen(filename, "w");

    fprintf(file, "%ld\n", space_size);

    for (size_t i = 0; i < space_size * space_size; i++) {
        fprintf(file, "%f\n", initial_heat_map[i]);
    }

    fclose(file);
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int process_count;
    MPI_Comm_size(MPI_COMM_WORLD, &process_count);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        read_data();
    }

    float start = MPI_Wtime();
    float total_trans_time = 0.0;

    MPI_Bcast(&space_size, 1, MPI_UINT64_T, 0, MPI_COMM_WORLD);
    h = 1.0 / space_size;

    total_trans_time += MPI_Wtime() - start;

    calculate_space_width_height(rank, process_count);

    scatter_initial_map(process_count, &total_trans_time);
    send_borders(rank, process_count, &total_trans_time);
    recv_borders(rank, process_count, &total_trans_time);

    for (size_t step = 0; step < 3200; step++) {
        simulate();

        send_borders(rank, process_count, &total_trans_time);
        recv_borders(rank, process_count, &total_trans_time);
    }

    gather_initial_map(process_count, &total_trans_time);

    float end = MPI_Wtime();

    if (rank == 0) {
        printf("Total time: %f\n", end - start);
        printf("Computation time: %f\n", end - start - total_trans_time);
        printf("Transmission time: %f\n", total_trans_time);
    }

    if (rank == 0) {
        write_data("output");
    }

    MPI_Finalize();
    return 0;
}
