'''
primeiro, precisamos de uma funcao que leia a entrada da matriz
e transforme isso em algo util. vamos transformar a matriz em 
um dicionario de adjacencia, que é mais facil de trabalhar no algoritmo
'''

{
    0: [1,3],
    1: [0,2],
    2: [1,3],
    3: [0,2]
}
'''
isso que o vertice 0 esta ligado aos vertices 1 e 3, o vertice
esta ligado aos vertices 0 e 2, e assim por diante


a matriz eh uma lista de listas
vamos linhas por linha(cada vertice)
se matriz[i][j] == 1, significa que existe aresta entre i e j
guardamos essas conexoes no dicionario
'''
def matriz_para_dicionario(matriz):
    n = len(matriz) # numero de vertices
    grafo = {}

    for i in range(n):
        grafo[i] = []
        for j in range(n):
            if matriz[i][j] == 1:
                grafo[i].append(j)
    return grafo


'''
todo vertice comeca com uma 'cor' baseada no numero de vizinhos
se o vertice 0 tem dois vizinhos, cor = 2
'''

def coloracao_inicial(grafo):
    cores = {} # o nome da cor e quantos vizinhos ela tem
    for v in grafo: # cores (dicionario) na posicao v, recebe tal grau
        cores[v] = len(grafo[v]) # grau do vertice
    return cores

'''
agora, a parte mais importante: o refinamento da coloracao. isso
eh o color refinement propriamente dito
aqui estamos criando uma etiqueta que representa a situacao de
cada vertice: sua cor atual e as cores de seus vizinhos
as tuplas (0, [0,0]) por exemplo
'''

def refinar_cores(grafo, cores_atuais):
    novos_rotulos = {}

    for v in grafo:
        vizinhos = grafo[v] # se v = 0, vizinhos = [1,3]
        # a variavel vizinhos agora guarda a lista de vertices conectados a v
        cores_vizinhos = sorted([cores_atuais[n] for n in vizinhos])
        # isso ajuda a garantir que a ordem dos vizinhos nao afete a comparacao
        # entre perfis, ex: [0,0,1]
        novos_rotulos[v] = (cores_atuais[v], tuple(cores_vizinhos))
    # por que tuple? porque listas nao podem ser usadas como chave em
    # dicionario, mas tuplas podem
    # ex: (2, (0,0)) cor do vertice 2, vizinhos com cores 0 e 0

    # agora precisamos trasformar essas tuplas em cores unicas
    # toda vez que uma nova tupla (perfil) aparece, ela ganha uma nova cor
    # (um nmero)
    # assim, os vertices vao se separando conforme seus vizinhos sao diferentes
    # ex: vertices com o mesmo perfil -> mesma cor
    # perfis diferentes -> cores diferentes
    mapa_de_rotulos = {}
    proxima_cor = 0
    novas_cores = {}

    for v in sorted(grafo): # garantir ordem consistente
        rotulo = novos_rotulos[v]
        if rotulo not in mapa_de_rotulos:
            mapa_de_rotulos[rotulo] = proxima_cor
            proxima_cor += 1
        novas_cores[v] = mapa_de_rotulos[rotulo]

    return novas_cores

"""
precisamos repetir o processo de refinamento ate as cores
pararem de mudar
repetimos o refinamento ate as cores estabilizarem(nao mudam de cor mais)
no final, comparamos a quantidade de vertices por cor
se forem diferentes-> grafos nao sao isomorfos
"""

def color_refinement(g1, g2):
    cores1 = coloracao_inicial(g1)
    cores2 = coloracao_inicial(g2)

    while True:
        novas_cores1 = refinar_cores(g1, cores1)
        novas_cores2 = refinar_cores(g2, cores2)

        if novas_cores1 == cores1 and novas_cores2 == cores2:
            break # estabilizou

        cores1 = novas_cores1
        cores2 = novas_cores2

    # contar quantos vertices tem cada cor
    from collections import Counter
    # counter serve para contar elementos repetidos
    # retorna um dicionario para cada item e sua quantidade
    contagem1 = Counter(cores1.values())
    contagem2 = Counter(cores2.values())

    return contagem1 == contagem2

'''
repetimos o refinamento ate as cores estabilizarem (nao mudam mais)
no final, comparamos a quantidade de vertices por cor
se forem diferentes -> grafos nao sao isomorfos

agora so falta transformar a entrada do arquivo para a matriz
'''

def ler_matriz_entrada(texto):
    linhas = texto.strip().split("\n") # separando as linhas por enter
    n = int(linhas[0]) # pega aquele primeiro numero la em cima 
    matriz = []
    for linha in linhas[1:n+1]:
        matriz.append([int(c) for c in linha.strip()])
        # limpamos os espacos iniciais e finais de novo por seguranca em cada linha
    return matriz
        

# funcao para ler os dois grafos do arquivo
def ler_dois_grafos_de_arquivo(caminho):
    with open(caminho, "r") as f:
        linhas = [linha.strip() for linha in f.readlines() if linha.strip() != ""]

    n = int(linhas[0]) # numero de vertices

    matriz1 = []
    for i in range(1, n + 1): 
        matriz1.append([int(c) for c in linhas[i]])

    matriz2 = []
    for i in range(n + 1, 2 * n + 1):
        matriz2.append([int(c) for c in linhas[i]])
    
    grafo1 = matriz_para_dicionario(matriz1)
    grafo2 = matriz_para_dicionario(matriz2)

    return grafo1, grafo2
    

if __name__ == "__main__":
    nome_arquivo = input("Nome do arquivo com os dois grafos: ")
    grafo1, grafo2 = ler_dois_grafos_de_arquivo(nome_arquivo)

    if color_refinement(grafo1, grafo2):
        print("\nResultado: POSSIVELMENTE isomorfos (color refinement nao distinguiu)")
    else:
        print("\nResultado: NAO SAO ISOMORFOS (color refinement distinguiu)")