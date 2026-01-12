#include <stdio.h>   // Para entrada/saída (printf, fopen, fclose, sscanf, perror)
#include <stdlib.h>  // Para alocação de memória (malloc, calloc, free, exit)
#include <string.h>  // Para manipulação de strings (fgets, sscanf)
#include <stdbool.h> // Para usar tipos booleanos (true, false)
#include <time.h>    // Para medir o tempo de execução (clock_t, clock, CLOCKS_PER_SEC)

// --- Estrutura para representar o Grafo ---
typedef struct {
    int num_vertices;
    int num_arestas;
    int **adj_matrix; // Matriz de adjacências (1 se há aresta, 0 caso contrário)
} Graph;

// --- Funções de Gerenciamento de Memória para o Grafo ---

// Aloca uma matriz de adjacências n x n, inicializando todos os elementos com 0.
// Retorna um ponteiro para a matriz alocada.
int **alloc_adj_matrix(int n) {
    int **mat = (int **)malloc(n * sizeof(int *));
    if (mat == NULL) {
        perror("Erro ao alocar memória para linhas da matriz");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < n; i++) {
        mat[i] = (int *)calloc(n, sizeof(int)); // calloc inicializa com 0
        if (mat[i] == NULL) {
            perror("Erro ao alocar memória para colunas da matriz");
            for (int k = 0; k < i; k++) { // Libera o que já foi alocado
                free(mat[k]);
            }
            free(mat);
            exit(EXIT_FAILURE);
        }
    }
    return mat;
}

// Libera a memória de uma matriz de adjacências alocada dinamicamente.
void free_adj_matrix(int **mat, int n) {
    if (mat == NULL) return;
    for (int i = 0; i < n; i++) {
        free(mat[i]);
    }
    free(mat);
}

// --- Função para Ler o Grafo do Arquivo DIMACS ---

// Lê um arquivo DIMACS de grafo e preenche uma estrutura Graph.
// Retorna um ponteiro para a estrutura Graph alocada e preenchida, ou NULL em caso de erro.
Graph *read_dimacs_graph(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("Erro ao abrir o arquivo DIMACS");
        return NULL;
    }

    Graph *graph = (Graph *)malloc(sizeof(Graph));
    if (graph == NULL) {
        perror("Erro ao alocar memória para a estrutura Graph");
        fclose(f);
        return NULL;
    }
    graph->adj_matrix = NULL; // Inicializa para segurança

    char line[256]; // Buffer para ler cada linha
    int u, v;       // Vértices da aresta

    while (fgets(line, sizeof(line), f)) {
        if (line[0] == 'c' || line[0] == '\n') {
            continue; // Linha de comentário ou vazia
        } else if (line[0] == 'p') {
            char problem_type[10]; // Para armazenar "edge" ou "col"
            // Usa sscanf para ler o tipo do problema e os números
            if (sscanf(line, "p %s %d %d", problem_type, &graph->num_vertices, &graph->num_arestas) != 3) {
                fprintf(stderr, "Erro: Linha 'p' mal formatada em %s: %s\n", filename, line);
                free(graph);
                fclose(f);
                return NULL;
            }
            // Verifica se o tipo é "edge" ou "col"
            if (strcmp(problem_type, "edge") != 0 && strcmp(problem_type, "col") != 0) {
                fprintf(stderr, "Erro: Tipo de problema desconhecido '%s' na linha 'p' em %s: %s\n", problem_type, filename, line);
                free(graph);
                fclose(f);
                return NULL;
            }
            // Se chegou aqui, a linha foi parseada corretamente
            graph->adj_matrix = alloc_adj_matrix(graph->num_vertices);
        } else if (line[0] == 'e') {
            if (sscanf(line, "e %d %d", &u, &v) != 2) {
                fprintf(stderr, "Erro ao parsear linha 'e' em %s\n", filename);
                free_adj_matrix(graph->adj_matrix, graph->num_vertices);
                free(graph);
                fclose(f);
                return NULL;
            }
            u--; // Ajustar para índice 0-baseado
            v--; // Ajustar para índice 0-baseado

            if (u >= 0 && u < graph->num_vertices && v >= 0 && v < graph->num_vertices) {
                graph->adj_matrix[u][v] = 1;
                graph->adj_matrix[v][u] = 1; // Grafo não direcionado
            } else {
                fprintf(stderr, "Aviso: Aresta inválida (%d, %d) lida do arquivo %s. Vértices fora do intervalo [1, %d].\n", u + 1, v + 1, filename, graph->num_vertices);
            }
        } else {
            fprintf(stderr, "Aviso: Linha de formato desconhecido ignorada: %s", line);
        }
    }

    fclose(f);
    return graph;
}

// --- Algoritmo First Fit para Coloração de Vértices ---

// Implementa o algoritmo First Fit para colorir um grafo.
// graph: Ponteiro para a estrutura Graph.
// colors: Um array de inteiros (alocado pelo chamador) onde as cores de cada vértice serão armazenadas.
//         colors[i] conterá a cor do vértice i.
// Retorna o número total de cores utilizadas.
int first_fit_coloring(Graph *graph, int *colors) {
    // Inicializa todas as cores dos vértices como 0 (não colorido)
    for (int i = 0; i < graph->num_vertices; i++) {
        colors[i] = 0;
    }

    int max_colors_used = 0; // Para rastrear o número máximo de cores atribuídas

    // Percorre cada vértice em ordem sequencial (0, 1, ..., N-1)
    for (int v = 0; v < graph->num_vertices; v++) {
        // `available_colors` é um array booleano para rastrear quais cores estão disponíveis para o vértice 'v'.
        // O tamanho é `graph->num_vertices + 1` porque as cores vão de 1 até `graph->num_vertices` (no pior caso).
        // `available_colors[0]` não é usado.
        bool *available_colors = (bool *)malloc((graph->num_vertices + 1) * sizeof(bool));
        if (available_colors == NULL) {
            perror("Erro ao alocar memória para available_colors");
            exit(EXIT_FAILURE);
        }

        // Inicialmente, todas as cores são consideradas disponíveis (de 1 até o máximo possível)
        for (int c = 1; c <= graph->num_vertices; c++) {
            available_colors[c] = true;
        }

        // Verifica os vizinhos do vértice 'v'
        for (int neighbor = 0; neighbor < graph->num_vertices; neighbor++) {
            // Se 'neighbor' é adjacente a 'v' E 'neighbor' já está colorido
            if (graph->adj_matrix[v][neighbor] == 1 && colors[neighbor] != 0) {
                // Marca a cor do vizinho como indisponível para 'v'
                if (colors[neighbor] <= graph->num_vertices) { // Prevenção de acesso inválido
                    available_colors[colors[neighbor]] = false;
                }
            }
        }

        // Encontra a menor cor disponível para o vértice 'v'
        int chosen_color = 1;
        while (chosen_color <= graph->num_vertices && !available_colors[chosen_color]) {
            chosen_color++;
        }

        // Atribui a cor encontrada ao vértice 'v'
        colors[v] = chosen_color;

        // Atualiza o número máximo de cores usadas até agora
        if (chosen_color > max_colors_used) {
            max_colors_used = chosen_color;
        }

        free(available_colors); // Libera a memória para o próximo vértice
    }

    return max_colors_used;
}

// --- Função Principal (main) para Testar ---
int main() {
    // Lista das instâncias de teste que você precisa rodar
    const char *instance_files[] = {
        "dsjc250.5", "dsjc500.1", "dsjc500.5", "dsjc500.9", "dsjc1000.1", "dsjc1000.5", "dsjc1000.9", 
        "r250.5", "r1000.1c", "r1000.5", "dsjr500.1c","dsjr500.5", "le450_25c", "le450.25d", 
        "flat300_28_0", "flat1000_50_0", "flat1000_60_0", "flat1000_76_0", "latin_square", "C2000.5", "C4000.5"
    };
    int num_instances = sizeof(instance_files) / sizeof(instance_files[0]);

    printf("--- Testando First Fit Algorithm ---\n\n");
    printf("%-20s %-10s %-10s %-15s\n", "Instancia", "Vertices", "Cores FF", "Tempo (s)");
    printf("----------------------------------------------------------\n");

    for (int i = 0; i < num_instances; i++) {
        const char *filename = instance_files[i];
        Graph *my_graph = read_dimacs_graph(filename);

        if (my_graph) {
            int *vertex_colors = (int *)malloc(my_graph->num_vertices * sizeof(int));
            if (vertex_colors == NULL) {
                perror("Erro ao alocar memória para cores dos vértices");
                free_adj_matrix(my_graph->adj_matrix, my_graph->num_vertices);
                free(my_graph);
                continue; // Pular para a próxima instância
            }

            clock_t start_time = clock();
            int num_colors_ff = first_fit_coloring(my_graph, vertex_colors);
            clock_t end_time = clock();
            double cpu_time_used = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

            printf("%-20s %-10d %-10d %-15.4f\n", filename, my_graph->num_vertices, num_colors_ff, cpu_time_used);

            free(vertex_colors); // Libera o array de cores
            free_adj_matrix(my_graph->adj_matrix, my_graph->num_vertices);
            free(my_graph); // Libera a estrutura Graph
        } else {
            fprintf(stderr, "Erro: Não foi possível carregar o grafo %s Pulando para o próximo.\n", filename);
        }
    }

    return 0;
}