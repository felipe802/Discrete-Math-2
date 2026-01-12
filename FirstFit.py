'''
grafo = {
    0: [1, 2],
    1: [0, 2],
    2: [0, 1, 3],
    3: [2]
}

  0
 / \
1---2
     \
      3
      
para cada vertice, ele:
1. Olha as cores ja atribuidas aos vizinhos
2. Marca quais cores ja estao ocupadas
3. Escolhe a menor cor livre(comecando do 1)
4. atribui essa cor ao vertice
'''

def first_fit_coloring(grafo):
    # dicionario para guardar as cores atribuidas aos vertices
    cores = {}

    # iteramos sobre os vertices em ordem crescente
    for vertice in sorted(range(len(grafo))): 
        # sorted(grafo) transforma as chaves do dicionario grafo em uma lista ordenada
        vizinhos = grafo[vertice] # lista de vizinhos do vertice atual
        cores_usadas = {cores[v] for v in vizinhos if v in cores}
        # para cada v que esta nos vizinhos, se esse vizinho ja tiver sido 
        #colorido, pegue a cor dele e coloque no conjunto cores_usadas
        
        # agora procuramos a menor cor (inteiro) que nao esta no conjunto de cores usadas
        cor = 1   
        # enquanto a cor atual ja estiver sendo usada por algum vizinho, tenta a proxima cor
        while cor in cores_usadas:
            cor += 1 # tenta a proxima cor
        cores[vertice] = cor
    return cores, max(cores.values())

'''
# exemplo
grafo = {
    0: [1, 2],
    1: [0, 2],
    2: [0, 1, 3],
    3: [2]
}

resultado = first_fit_coloring(grafo)

for v in sorted(resultado):
    print(f"Vertice {v} -> cor {resultado[v]}")
'''

def ler_instancia_dimacs(caminho_arquivo):
    with open(caminho_arquivo, 'r') as arquivo:
        linhas = arquivo.readlines()

    n_vertices = 0
    arestas = []

    for linha in linhas:
        if linha.startswith("c"):
            continue # ignora comentarios
        elif linha.startswith("p"):
            partes = linha.strip().split()
            n_vertices = int(partes[2])
            # num_arestas = int(partes[3]) # eu posso guardar se quiser
        elif linha.startswith('e'):
            _, u, v = linha.strip().split()
            # cria uma tupla com os dois vertices da aresta
            arestas.append((int(u)-1, int(v)-1)) # convertendo para base 0
        
    # grafo como lista de adjacencia
    grafo = [[] for _ in range(n_vertices)]
    for u, v in arestas:
        grafo[u].append(v)
        grafo[v].append(u) # grafo nao direcionado

    return grafo, n_vertices


# bloco principal para ler as instancias, aplicar o algoritmo e exibir a tabela
import time

def main():
    instance_files = [

        "dsjc250.5", "dsjc500.1", "dsjc500.5", "dsjc500.9", "dsjc1000.1", "dsjc1000.5", "dsjc1000.9", 
        "r250.5", "r1000.1c", "r1000.5", "dsjr500.1c", "dsjr500.5", "le450_25c", "le450.25d", 
        "flat300_28_0", "flat1000_50_0", "flat1000_60_0", "flat1000_76_0", "latin_square", "C2000.5", "C4000.5"
    ]

    print("--- Testando First Fit Algorithm ---\n")
    print(f"{'Instancia':<20} {'Vertices':<10} {'Cores':<10} {'Tempo (s)':<15}")
    print("-" * 58)

    for filename in instance_files:
        try:
            grafo, n_vertices = ler_instancia_dimacs(filename)
            inicio = time.process_time()
            _, num_cores = first_fit_coloring(grafo)
            fim = time.process_time()
            tempo = fim - inicio
            print(f"{filename:<20} {n_vertices:<10} {num_cores:<10} {tempo:<15.4f}")
        except Exception as e:
            print(f"{filename:<20} Erro ao processar: {e}")

if __name__ == "__main__":
    main()