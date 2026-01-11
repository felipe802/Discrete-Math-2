#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int **alloc_matrix(int n, int m) {
    int **mat = malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++)
        mat[i] = calloc(m, sizeof(int));
    return mat;
}

void free_matrix(int **mat, int n) {
    for (int i = 0; i < n; i++)
        free(mat[i]);
    free(mat);
}

int read_matrix(FILE *f, int ***mat, int n, int m) {
    *mat = alloc_matrix(n, m);
    char buffer[1024];
    for (int i = 0; i < n; i++) {
        if (fscanf(f, "%s", buffer) != 1) {
            printf("Erro ao ler linha %d\n", i);
            return 0;
        }
        for (int j = 0; j < m; j++)
            (*mat)[i][j] = buffer[j] - '0';
    }
    return 1;
}

void init_colors(int **incidence, int n, int m, int *color) {
    for (int i = 0; i < n; i++) {
        int degree = 0;
        for (int j = 0; j < m; j++)
            degree += incidence[i][j];
        color[i] = degree;
    }
}

void refine_colors(int **incidence, int n, int m, int *color, int *new_color) {
    for (int i = 0; i < n; i++) {
        int *signature = calloc(n, sizeof(int));
        for (int j = 0; j < m; j++) {
            if (incidence[i][j]) {
                for (int k = 0; k < n; k++) {
                    if (k != i && incidence[k][j])
                        signature[color[k]]++;
                }
            }
        }
        int hash = 0;
        for (int c = 0; c < n; c++)
            if (signature[c] > 0)
                hash = hash * 31 + c * signature[c];
        new_color[i] = hash;
        free(signature);
    }
}

int color_refinement(int **inc1, int **inc2, int n, int m) {
    int *color1 = malloc(n * sizeof(int));
    int *color2 = malloc(n * sizeof(int));
    int *new_color1 = malloc(n * sizeof(int));
    int *new_color2 = malloc(n * sizeof(int));

    init_colors(inc1, n, m, color1);
    init_colors(inc2, n, m, color2);

    int changed = 1;
    while (changed) {
        refine_colors(inc1, n, m, color1, new_color1);
        refine_colors(inc2, n, m, color2, new_color2);
        changed = 0;
        for (int i = 0; i < n; i++) {
            if (color1[i] != new_color1[i]) changed = 1;
            color1[i] = new_color1[i];
            color2[i] = new_color2[i];
        }
    }

    int *freq1 = calloc(n * n, sizeof(int));
    int *freq2 = calloc(n * n, sizeof(int));
    for (int i = 0; i < n; i++) {
        freq1[color1[i] % (n * n)]++;
        freq2[color2[i] % (n * n)]++;
    }
    int result = 1;
    for (int i = 0; i < n * n; i++) {
        if (freq1[i] != freq2[i]) {
            result = 0;
            break;
        }
    }

    free(color1); free(color2); free(new_color1); free(new_color2);
    free(freq1); free(freq2);
    return result;
}

int main() {
    FILE *f = fopen("instancias isomorfismo.txt", "r");
    if (!f) {
        printf("Erro ao abrir arquivo.\n");
        return 1;
    }

    int instance = 1;
    while (1) {
        int n;
        if (fscanf(f, "%d", &n) != 1) break;

        int **inc1 = NULL, **inc2 = NULL;
        if (!read_matrix(f, &inc1, n, n)) break;
        if (!read_matrix(f, &inc2, n, n)) {
            free_matrix(inc1, n);
            break;
        }

        clock_t start = clock();
        int result = color_refinement(inc1, inc2, n, n);
        clock_t end = clock();
        double cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;

        printf("%d) n = %d %s %.3f\n", instance, n, result ? "+++" : "---", cpu_time);

        free_matrix(inc1, n);
        free_matrix(inc2, n);
        instance++;
    }

    fclose(f);
    return 0;
}
