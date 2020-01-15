#include <stdlib.h>
#include <stdio.h>
#include <time.h>

typedef unsigned short u16;
typedef unsigned char boolean;

boolean exists[10000][10000] = { 0 };

int main() {
    srand(time(NULL));

    FILE *file = fopen("input", "w");

    const u16 vertex_count = 1000;
    const u16 edge_count = vertex_count * 4;

    fprintf(file, "%d\n", vertex_count);
    fprintf(file, "%d\n", edge_count);
    for (u16 i = 0; i < edge_count; i++) {
        u16 u, v;
        do {
            u = rand() % vertex_count;
            v = rand() % vertex_count;
        } while (u == v || exists[u][v]);
        exists[u][v] = 1;
        fprintf(file, "%d %d\n", u, v);
    }

    fclose(file);
    return 0;
}
