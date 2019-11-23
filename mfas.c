#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>

typedef unsigned short u16;

u16 random_between(u16 a, u16 b) {
    double v = (double)rand() * (b + 1 - a)/ ((long)RAND_MAX + 1);
    u16 result = floor(v) + a;
    if (result > b)
        return b;
    if (result < a)
        return a;
    return result;
}

void permutation(u16 *a, size_t n) {
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

size_t vertex_count, edge_count, result_edge_count;
struct edge edges[1 << 16];
struct edge result_edges[1 << 16];
u16 A[1 << 16];
u16 inverted_A[1 << 16];

void read_input() {
    scanf("%ld", &vertex_count);
    scanf("%ld", &edge_count);
    for (u16 i = 0; i < edge_count; i++) {
        int u, v;
        scanf("%d %d", &u, &v);
        struct edge e = {u, v};
        edges[i] = e;
    }
}

void print_debug() {
    printf("%ld\n", vertex_count);
    printf("%ld\n", edge_count);
    for (u16 i = 0; i < edge_count; i++) {
        struct edge e = edges[i];
        printf("%d %d\n", e.u, e.v);
    }
}

void compute_inverted_A() {
    for (u16 i = 0; i < vertex_count; i++) {
        inverted_A[A[i]] = i;
    }
}

void print(u16 *a, size_t N) {
    for (u16 i = 0; i < N; i++) {
        printf("%d ", a[i]);
    }
    printf("\n");
}

size_t max(size_t a, size_t b) {
    return a > b ? a : b;
}

size_t minimum_feedback_arc_set(size_t prev_min) {
    for (u16 i = 0; i < vertex_count; i++) {
        A[i] = i;
    }
    permutation(A, vertex_count);
    compute_inverted_A();

    print(A, vertex_count);

    size_t forward_count = 0;
    size_t backward_count = 0;
    for (u16 k = 0; k < vertex_count; k++) {
        struct edge e = edges[k];
        u16 index_u = inverted_A[e.u];
        u16 index_v = inverted_A[e.v];

        if (index_v > index_u)
            forward_count++;
        else 
            backward_count++;
    }

    size_t new_min = edge_count - max(forward_count, backward_count);
    if (new_min < prev_min) {
        if (forward_count >= backward_count) {
            result_edge_count = 0;
            for (u16 k = 0; k < edge_count; k++) {
                struct edge e = edges[k];
                u16 index_u = inverted_A[e.u];
                u16 index_v = inverted_A[e.v];

                if (index_v > index_u) {
                    result_edges[result_edge_count++] = e;
                }
            }
        }
        else {
            result_edge_count = 0;
            for (u16 k = 0; k < edge_count; k++) {
                struct edge e = edges[k];
                u16 index_u = inverted_A[e.u];
                u16 index_v = inverted_A[e.v];

                if (index_u > index_v) {
                    result_edges[result_edge_count++] = e;
                }
            }
        }
        return new_min;
    }
    else {
        return prev_min;
    }
}

void print_result() {
    for (u16 k = 0; k < result_edge_count; k++) {
        struct edge e = result_edges[k];
        printf("%d %d\n", e.u, e.v);
    }
}

int main() {
    srand(time(NULL));
    read_input();
    // print_debug();
    size_t min = minimum_feedback_arc_set(edge_count);
    printf("min: %ld\n", min);
    print_result();
    return 0;
}
