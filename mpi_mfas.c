#include <mpi.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>

#define MAX_PROCESSES 1000
#define TOTAL_SIMULATION_COUNT 10

typedef unsigned short u16;
typedef unsigned int usize;

u16 random_between(u16 a, u16 b) {
    double v = (double)rand() * (b + 1 - a)/ ((long)RAND_MAX + 1);
    u16 result = floor(v) + a;
    if (result > b)
        return b;
    if (result < a)
        return a;
    return result;
}

void permutation(u16 *a, usize n) {
    for (u16 i = n - 1; i >= 1; i--) {
        u16 j = random_between(0, i);
        u16 tmp = a[i];
        a[i] = a[j];
        a[j] = tmp;
    }
}

struct edge {
    u16 u, v;
};

usize vertex_count, edge_count;
struct edge edges[1 << 16];
struct edge result_edges[1 << 16];
u16 A[1 << 16];
u16 inverted_A[1 << 16];

int seeds[MAX_PROCESSES];
int seed;
usize minimum_results[MAX_PROCESSES];

void read_input() {
    scanf("%d", &vertex_count);
    scanf("%d", &edge_count);
    for (u16 i = 0; i < edge_count; i++) {
        int u, v;
        scanf("%d %d", &u, &v);
        struct edge e = {u, v};
        edges[i] = e;
    }
}

void compute_inverted_A() {
    for (u16 i = 0; i < vertex_count; i++) {
        inverted_A[A[i]] = i;
    }
}

usize max(usize a, usize b) {
    return a > b ? a : b;
}

usize minimum_feedback_arc_set(usize prev_min) {
    for (u16 i = 0; i < vertex_count; i++) {
        A[i] = i;
    }
    permutation(A, vertex_count);
    compute_inverted_A();

    usize forward_count = 0;
    usize backward_count = 0;
    for (u16 k = 0; k < vertex_count; k++) {
        struct edge e = edges[k];
        u16 index_u = inverted_A[e.u];
        u16 index_v = inverted_A[e.v];

        if (index_v > index_u)
            forward_count++;
        else 
            backward_count++;
    }

    usize new_min = edge_count - max(forward_count, backward_count);
    if (new_min < prev_min) {
        if (forward_count >= backward_count) {
            usize count = 0;
            for (u16 k = 0; k < edge_count; k++) {
                struct edge e = edges[k];
                u16 index_u = inverted_A[e.u];
                u16 index_v = inverted_A[e.v];

                if (index_v < index_u) {
                    result_edges[count++] = e;
                }
            }
        }
        else {
            usize count = 0;
            for (u16 k = 0; k < edge_count; k++) {
                struct edge e = edges[k];
                u16 index_u = inverted_A[e.u];
                u16 index_v = inverted_A[e.v];

                if (index_u < index_v) {
                    result_edges[count++] = e;
                }
            }
        }
        return new_min;
    }
    else {
        return prev_min;
    }
}

void print_result(int rank,usize count) {
    for (u16 k = 0; k < count; k++) {
        struct edge e = result_edges[k];
        printf("%d: %d %d\n", rank, e.u, e.v);
    }
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int process_count;
    MPI_Comm_size(MPI_COMM_WORLD, &process_count);

    // Read from input file and broadcast, include seeds
    if (rank == 0) {
        read_input();
        srand(time(NULL));
        for (int i = 0; i < process_count; i++) 
            seeds[i] = rand();
    }
    MPI_Bcast(&vertex_count, 1, MPI_UINT32_T, 0, MPI_COMM_WORLD);
    MPI_Bcast(&edge_count, 1, MPI_UINT32_T, 0, MPI_COMM_WORLD);
    MPI_Bcast(edges, edge_count * 2, MPI_UINT16_T, 0, MPI_COMM_WORLD);

    MPI_Scatter(seeds, 1, MPI_INT, &seed, 1, MPI_INT, 0, MPI_COMM_WORLD);
    srand(seed);

    usize minimum = edge_count;
    for (int n = 0; n < (TOTAL_SIMULATION_COUNT / process_count); n++) {
        minimum = minimum_feedback_arc_set(minimum);
    }

    MPI_Gather(&minimum, 1, MPI_INT, minimum_results, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int winner_rank = 0;
    usize winner_min = minimum;
    if (rank == 0) {
        for (int i = 1; i < process_count; i++) {
            if (minimum_results[i] < winner_min) {
                winner_min = minimum_results[i];
                winner_rank = i;
            }
        }
    }
    MPI_Bcast(&winner_rank, 1, MPI_INT, 0, MPI_COMM_WORLD);

    printf("Winner rank: %d\n", winner_rank);

    if (rank == winner_rank) {
        MPI_Send(result_edges, minimum * 2, MPI_UINT16_T, 0, 111, MPI_COMM_WORLD);
    }

    if (rank == 0) {
        MPI_Status status;
        MPI_Recv(result_edges, winner_min * 2, MPI_UINT16_T, winner_rank, 111, MPI_COMM_WORLD, &status);
        printf("Minimum Feedback Arc Set: %d\n", winner_min);
        for (u16 k = 0; k < winner_min; k++) {
            struct edge e = result_edges[k];
            printf("%d %d\n", e.u, e.v);
        }
    }

    MPI_Finalize();
    return 0;
}
