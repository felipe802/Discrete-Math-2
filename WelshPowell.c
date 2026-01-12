#include <stdio.h>   // Para entrada/saída (printf, fopen, fclose, sscanf, perror)
#include <stdlib.h>  // Para alocação de memória (malloc, calloc, free, exit, qsort)
#include <string.h>  // Para manipulação de strings (fgets, sscanf, strcmp)
#include <stdbool.h> // Para usar tipos booleanos (true, false)
#include <time.h>    // Para medir o tempo de execução (clock_t, clock, CLOCKS_PER_SEC)

// --- Estrutura para representar o Grafo ---
typedef struct {
    int num_vertices;
    int num_arestas;
    int **adj_matrix; // Matriz de adjacências (1 se há aresta, 0 caso contrário)
} Graph;

// --- Estrutura auxiliar para Welsh-Powell (Vértice e seu Grau) ---
typedef struct {
    int id;     // ID do vértice (0 a N-1)
    int degree; // Grau do vértice
} VertexDegree;

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
            char problem_type[10]; // To store "edge" or "col"
            // Use sscanf to read the problem type string, then the numbers
            if (sscanf(line, "p %s %d %d", problem_type, &graph->num_vertices, &graph->num_arestas) != 3) {
                fprintf(stderr, "Erro: Linha 'p' mal formatada em %s: %s\n", filename, line);
                free(graph);
                fclose(f);
                return NULL;
            }
            // Now check if problem_type is "edge" or "col"
            if (strcmp(problem_type, "edge") != 0 && strcmp(problem_type, "col") != 0) {
                fprintf(stderr, "Erro: Tipo de problema desconhecido '%s' na linha 'p' em %s: %s\n", problem_type, filename, line);
                free(graph);
                fclose(f);
                return NULL;
            }
            // If we reach here, the line was parsed correctly.
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

// --- Funções Auxiliares para Welsh-Powell ---

// Função de comparação para qsort: ordena VertexDegree em ordem decrescente de grau.
int compare_vertex_degree(const void *a, const void *b) {
    return ((VertexDegree *)b)->degree - ((VertexDegree *)a)->degree;
}

// Calcula o grau de todos os vértices do grafo.
// graph: Ponteiro para a estrutura Graph.
// degrees: Um array de VertexDegree (alocado pelo chamador) onde os IDs e graus serão armazenados.
void calculate_all_degrees(Graph *graph, VertexDegree *degrees) {
    for (int i = 0; i < graph->num_vertices; i++) {
        degrees[i].id = i;
        degrees[i].degree = 0;
        for (int j = 0; j < graph->num_vertices; j++) {
            if (graph->adj_matrix[i][j] == 1) {
                degrees[i].degree++;
            }
        }
    }
}

// --- Algoritmo Welsh-Powell para Coloração de Vértices ---

// Implementa o algoritmo Welsh-Powell para colorir um grafo.
// graph: Ponteiro para a estrutura Graph.
// colors: Um array de inteiros (alocado pelo chamador) onde as cores de cada vértice serão armazenadas.
//         colors[i] conterá a cor do vértice i.
// Retorna o número total de cores utilizadas.
int welsh_powell_coloring(Graph *graph, int *colors) {
    // Inicializa todas as cores dos vértices como 0 (não colorido)
    for (int i = 0; i < graph->num_vertices; i++) {
        colors[i] = 0;
    }

    // Passo 1: Calcular os graus de todos os vértices
    VertexDegree *all_vertices_degrees = (VertexDegree *)malloc(graph->num_vertices * sizeof(VertexDegree));
    if (all_vertices_degrees == NULL) {
        perror("Erro ao alocar memória para all_vertices_degrees");
        exit(EXIT_FAILURE);
    }
    calculate_all_degrees(graph, all_vertices_degrees);

    // Ordena os vértices em ordem decrescente de grau
    qsort(all_vertices_degrees, graph->num_vertices, sizeof(VertexDegree), compare_vertex_degree);

    int current_color = 1; // Começa com a primeira cor
    int colored_count = 0; // Conta quantos vértices já foram coloridos

    // Array para controlar quais vértices já foram definitivamente coloridos
    bool *is_colored = (bool *)calloc(graph->num_vertices, sizeof(bool));
    if (is_colored == NULL) {
        perror("Erro ao alocar memória para is_colored");
        free(all_vertices_degrees);
        exit(EXIT_FAILURE);
    }

    // Continua enquanto houver vértices não coloridos
    while (colored_count < graph->num_vertices) {
        // Passo 2: Selecionar o vértice de maior grau não colorido
        int start_vertex_id = -1;
        for (int i = 0; i < graph->num_vertices; i++) {
            if (!is_colored[all_vertices_degrees[i].id]) {
                start_vertex_id = all_vertices_degrees[i].id;
                break; // Encontrou o vértice não colorido de maior grau
            }
        }

        if (start_vertex_id == -1) {
            // Todos os vértices foram coloridos, sai do loop (deveria ser pego por colored_count < num_vertices)
            break;
        }

        // Passo 3: Colorir o vértice selecionado com a cor ativa
        colors[start_vertex_id] = current_color;
        is_colored[start_vertex_id] = true;
        colored_count++;

        // Agora, encontre outros vértices não adjacentes ao vértice_inicial
        // que ainda não foram coloridos, e os adicione ao conjunto V'
        // Em Welsh Powell, não precisamos explicitamente de V'.
        // Iteramos pelos vértices ordenados por grau novamente,
        // mas consideramos apenas aqueles que não são adjacentes ao 'start_vertex_id'
        // e que ainda não foram coloridos.
        for (int i = 0; i < graph->num_vertices; i++) {
            int current_v_id = all_vertices_degrees[i].id;

            // Se o vértice atual não foi colorido E não é adjacente ao vértice inicial colorado
            if (!is_colored[current_v_id] && graph->adj_matrix[start_vertex_id][current_v_id] == 0) {
                // Verificar se current_v_id é adjacente a algum VERTICE JÁ COLORIDO COM current_color.
                // Esta é a parte crucial e aprimorada do Welsh-Powell, para garantir que os
                // vértices não adjacentes ao start_vertex_id e entre si possam usar a mesma cor.
                bool can_color_with_current = true;
                for (int j = 0; j < graph->num_vertices; j++) {
                    if (graph->adj_matrix[current_v_id][j] == 1 && colors[j] == current_color) {
                        can_color_with_current = false;
                        break; // Não pode usar esta cor, pois é adjacente a um vizinho já com esta cor
                    }
                }

                if (can_color_with_current) {
                    colors[current_v_id] = current_color;
                    is_colored[current_v_id] = true;
                    colored_count++;
                }
            }
        }
        current_color++; // Passo 4: Próxima cor para os vértices restantes não coloridos
    }

    free(all_vertices_degrees);
    free(is_colored);

    return current_color - 1; // Retorna o total de cores usadas
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

    printf("--- Comparação de Algoritmos de Coloração de Grafos ---\n\n");
    printf("%-20s %-10s %-10s %-15s %-10s %-15s\n", "Instancia", "Vertices", "Cores FF", "Tempo FF (s)", "Cores WP", "Tempo WP (s)");
    printf("--------------------------------------------------------------------------------------\n");

    for (int i = 0; i < num_instances; i++) {
        const char *filename = instance_files[i];
        Graph *my_graph = read_dimacs_graph(filename);

        if (my_graph) {
            int *vertex_colors_ff = (int *)malloc(my_graph->num_vertices * sizeof(int));
            int *vertex_colors_wp = (int *)malloc(my_graph->num_vertices * sizeof(int));

            if (vertex_colors_ff == NULL || vertex_colors_wp == NULL) {
                perror("Erro ao alocar memória para cores dos vértices");
                free_adj_matrix(my_graph->adj_matrix, my_graph->num_vertices);
                free(my_graph);
                if (vertex_colors_ff) free(vertex_colors_ff);
                if (vertex_colors_wp) free(vertex_colors_wp);
                continue; // Pular para a próxima instância
            }

            // --- Executar First Fit ---
            clock_t start_time_ff = clock();
            int num_colors_ff = first_fit_coloring(my_graph, vertex_colors_ff);
            clock_t end_time_ff = clock();
            double cpu_time_ff = ((double)(end_time_ff - start_time_ff)) / CLOCKS_PER_SEC;

            // --- Executar Welsh-Powell ---
            clock_t start_time_wp = clock();
            int num_colors_wp = welsh_powell_coloring(my_graph, vertex_colors_wp);
            clock_t end_time_wp = clock();
            double cpu_time_wp = ((double)(end_time_wp - start_time_wp)) / CLOCKS_PER_SEC;

            printf("%-20s %-10d %-10d %-15.4f %-10d %-15.4f\n",
                   filename, my_graph->num_vertices, num_colors_ff, cpu_time_ff,
                   num_colors_wp, cpu_time_wp);

            free(vertex_colors_ff); // Libera o array de cores FF
            free(vertex_colors_wp); // Libera o array de cores WP
            free_adj_matrix(my_graph->adj_matrix, my_graph->num_vertices);
            free(my_graph); // Libera a estrutura Graph
        } else {
            fprintf(stderr, "Erro: Não foi possível carregar o grafo %s. Pulando para o próximo.\n", filename);
        }
    }

    return 0;
}