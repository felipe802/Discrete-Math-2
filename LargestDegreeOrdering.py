'''
os vertices sao coloridos na ordem decrescente de grau (quantidade 
de vizinhos). A ideia eh comecar pelos vertices mais "conectados",
porque sao mais restritos nas escolhas de cores
'''

def read_dimacs_graph(filename):
    try:
        with open(filename, 'r') as f:
            num_vertices = 0
            num_arestas = 0
            adj_matrix = []
            
            for line in f:
                if line.startswith('c') or line.strip() == '':
                    continue # comment or empty line
                elif line.startswith('p'):
                    parts = line.strip().split()
                    if len(parts) != 4:
                        print(f"Erro: linha 'p' mal formatada: {line}")
                        return None
                    problem_type, num_vertices, num_arestas = parts[1], int(parts[2]), int(parts[3])
                    if problem_type not in ('edge', 'col'):
                        print(f"Erro: tipo de problema invalido: {problem_type}")
                        return None
                    # cria a matriz de adjacencia porque precisamos verificas as vizinhancas
                    # se nao precisassemos dos vizinhos bastava calcular diretamente pelo arquivo .col
                    # verificar cada vizinho toda vez eh uma operacao muito lenta
                    adj_matrix = [[0 for _ in range(num_vertices)] for _ in range(num_vertices)]
                elif line.startswith("e"):
                    parts = line.strip().split()
                    if len(parts) != 3:
                        print(f"Erro ao processar linha 'e': {line}")
                        return None
                    u, v = int(parts[1])-1, int(parts[2])-1 # adjust to base index 0
                    if 0 <= u <= num_vertices and 0 <= v <= num_vertices:
                        adj_matrix[u][v] = 1
                        adj_matrix[v][u] = 1
                    else:
                        print(f"Aviso: aresta invalida ({u+1}, {v+1}) fora do intervalo [1, {num_vertices}]")
                else:
                    print(f"Aviso: linha ignorada: {line.strip()}")
            return {'num_vertices': num_vertices, 'adj_matrix': adj_matrix}
    except FileNotFoundError:
        print(f"Erro ao abrir arquivo: {filename}")
        return None

def calculate_all_degrees(adj_matrix):
    degress = []
    for i in range(len(adj_matrix)): # how many lines, how many nodes in matrix
        degree = sum(adj_matrix[i]) # como cada linha sao os vizinhos daquele vertice
        # isso conta quantas arestas estao ligadas ao vertice i
        degress.append({'id': i, 'degree': degree})
        # degree eh sobrescrito mas a passada anterior ja foi salva
    return degress
    # aqui temos qual vertice tem qual grau
    # podemos entao reordenar por grau, mantendo os indices dos vertices certos

def compare_by_degree_desc(vertex): # wait receive a dict
    return -vertex['degree'] # negativo = ordem descrescente
'''
# essas funcao eh usaada para ordenar os vertices em ordem descrescente de grau
# -grau eh fazer o inverso da ordem crescente
exemplo:
entrada = [{'id': 0, 'degree': 2}, {'id':1, 'degree':5}, {'id':2, 'degree':3}]
entrada.sort(key=compare_by_degree_desc) python chama automaticamente a funcao para cada elemento(dict) da lista,
passando cada dicionario como argumento
resultado: [{'id': 1, 'degree': 5}, {'id': 2, 'degree': 3}, {'id': 0, 'degree': 2}]

É como se o Python fizesse isso por trás dos panos:
compare_by_degree_desc({'id': 0, 'degree': 3})  # retorna -3
compare_by_degree_desc({'id': 1, 'degree': 5})  # retorna -5
compare_by_degree_desc({'id': 2, 'degree': 1})  # retorna -1
'''

def largest_degree_ordering_coloring(graph):
    # graph eh um dicionario recebido o retorno do read_dimacs_graph line 43
    num_vertices = graph['num_vertices']
    adj_matrix = graph['adj_matrix']
    colors = [0] * num_vertices # create a colors list to keep each
    # node color. all start with 0. That means not colored yet

    # calcular graus e ordenar
    vertex_degrees = calculate_all_degrees(adj_matrix) # this is a list of dicts
    # calc each node degree and save in a dict list
    # [{'id': 0, 'degree': 3}, {'id': 1, 'degree': 5}, ...]
    vertex_degrees.sort(key=compare_by_degree_desc) 
    # ordered by decrescente order

    max_colors_used = 0
    # the biggest number of usef colors, the total

    for v_info in vertex_degrees:
        v = v_info['id']
        # v is the vertex number
        avaiable_colors = [True] * (num_vertices + 1) # cores de 1 ate N
        # a list of flags of avaiable colors found by index

        for neighbor in range(num_vertices):
            # verify all neighbors of v vertex
            if adj_matrix[v][neighbor] == 1 and colors[neighbor] != 0: # already colored
                used_color = colors[neighbor]
                if used_color <= num_vertices:
                    avaiable_colors[used_color] = False # everything in the list should be True
                    # this color have been used = False 
        
        chosen_color = 1
        while chosen_color <= num_vertices and not avaiable_colors[chosen_color]:
            chosen_color += 1
        # find the smallest color avaiable (first fit)
        # skip the Falses one

        colors[v] = chosen_color
        # colors position v receive the smallest color
        if chosen_color > max_colors_used:
            # update the biggest color if bigger than the last one
            max_colors_used = chosen_color

    return colors, max_colors_used
# return the final each color list and the total used colors

# mainly function that run the files
import time

def main():
    instance_files = [
        "dsjc250.5", "dsjc500.1", "dsjc500.5", "dsjc500.9", "dsjc1000.1", "dsjc1000.5", "dsjc1000.9",
        "r250.5", "r1000.1c", "r1000.5", "dsjr500.1c", "dsjr500.5", "le450_25c", "le450.25d", 
        "flat300_28_0", "flat1000_50_0", "flat1000_60_0", "flat1000_76_0", "latin_square", "C2000.5", "C4000.5"
    ]

    print("--- Coloração de Grafos com LDO ---\n")
    print(f"{'Instancia':<20} {'Vertices':<10} {'Cores LDO':<10} {'Tempo (s)':<10}")
    # :<20 align by left 20 columns (I could use this in my old_game)
    print("-" * 60)

    for filename in instance_files:
        graph = read_dimacs_graph(filename)
        if graph:
            start = time.process_time()
            colors, used = largest_degree_ordering_coloring(graph)
            end = time.process_time()
            tempo = end - start
            print(f"{filename:<20} {graph['num_vertices']:<10} {used:<10} {tempo:<10.4f}")
        else:
            print(f"Erro ao processar: {filename}")
        
if __name__ == "__main__":
    main()
    # run only if directly asked, not when imported as module to another script
    # when does it happen?