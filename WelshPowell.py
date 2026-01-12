'''
1. Calcular o grau de cada vertice
o grau de cada vertice eh o tamanho da sua lista de vizinhos
graus = {v: len(vizinhos) for v, vizinhos in grafo.items()}
# resultado: {0: 2, 1: 2, 2: 3, 3: 1}

2. Ordenar os vértices por grau decrescente
ordem_vertices = sorted(graus, key=lamba v: graus[v], reverse=True)
# resultado: [2, 0, 1, 3]

3. atribuir cores aos vertices nessa ordem, sempre tentando usar a menor cor 
possivel que nao seja usada pelos vizinhos ja coloridos


4. se possivel, pinta mais de um vertice com a mesma cor, desde que eles nao sejam 
adjacentes entre si

Vamos supor que temos um grafo representado por lista de adjacência:
grafo = {
    0: [1, 2],
    1: [0, 2],
    2: [0, 1, 3],
    3: [2]
}
'''
import os
import time

def ler_instancia_dimacs(caminho_arquivo):
    with open(caminho_arquivo, 'r') as arquivo:
        linhas = arquivo.readlines()

    n_vertices = 0
    arestas = []

    for linha in linhas:
        if linha.startswith("c"):
            continue  # Comentário
        elif linha.startswith("p"):
            partes = linha.strip().split()
            n_vertices = int(partes[2])  # pega o número de vértices
        elif linha.startswith("e"):
            _, u, v = linha.strip().split()
            arestas.append((int(u)-1, int(v)-1))  # ajusta para base 0

    # cria a lista de adjacência
    grafo = [[] for _ in range(n_vertices)]
    for u, v in arestas:
        grafo[u].append(v)
        grafo[v].append(u)  # grafo não direcionado

    return grafo, n_vertices


def welsh_powell_coloring(grafo):
    # 1. Calcular o grau de cada vertice
    graus = {v: len(grafo[v]) for v in range(len(grafo))}

    # 2. ordenar
    ordem_vertices = sorted(graus, key=lambda v: graus[v], reverse=True)
    cores = {} 
    cor_atual = 1 # comecamos com a cor 1

    # 3. enquanto ainda houver vertices nao coloridos
    while len(cores) < len(grafo):
        for v in ordem_vertices:
            if v not in cores:
                # se nenhum vizinho tem a cor atual, podemos pintar
                if all(cores.get(vizinho) != cor_atual for vizinho in grafo[v]):
                    cores[v] = cor_atual
        cor_atual += 1 # incrementa pra proxima 
    return cores, max(cores.values())


def main():
    nomes_instancias = [
        "dsjc250.5", "dsjc500.1", "dsjc500.5", "dsjc500.9", "dsjc1000.1", "dsjc1000.5", "dsjc1000.9", 
        "r250.5", "r1000.1c", "r1000.5", "dsjr500.1c", "dsjr500.5", "le450_25c", "le450.25d", 
        "flat300_28_0", "flat1000_50_0", "flat1000_60_0", "flat1000_76_0", "latin_square", "C2000.5", "C4000.5"
    ]

    print("\n--- Testando Welsh-Powell Algorithm ---\n")
    print(f"{'Instância':<20} {'Vértices':<10} {'Cores':<10} {'Tempo (s)':<15}")
    print("-" * 58)

    for nome in nomes_instancias:
        try:
            grafo, n_vertices = ler_instancia_dimacs(nome)
            inicio = time.process_time()
            _, num_cores = welsh_powell_coloring(grafo)
            fim = time.process_time()
            tempo = fim - inicio
            print(f"{nome:<20} {n_vertices:<10} {num_cores:<10} {tempo:<15.4f}")
        except Exception as e:
            print(f"{nome:<20} Erro ao processar: {e}")

if __name__ == "__main__":
    main()