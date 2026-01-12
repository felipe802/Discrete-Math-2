# criar uma image no formato PBM contendo um texto
from PIL import Image, ImageDraw, ImageFont
# mostra imagem utilizando matplotlib
from PIL import Image
import matplotlib.pyplot as plt

'''
n = int(input("rows n: ")); m = int(input("cols m: "))
matrix = [[0] * m for _ in range(n)] 
for i in range(n): 
  print(f"row {i}: ")
  linha = input().split() # '2', '5', '8'...
  for j in range(m):
    matrix[i][j] = int(linha[j])
'''

# ler uma image no formato PBM
def leiaImagemPBM(filename):
    with open(filename, 'r') as file:
        lines = file.readlines()

    # Verifica se o arquivo é uma imagem PBM válida
    if lines[0].strip() != 'P1':
        raise ValueError('O arquivo não é uma imagem PBM válida.')

    # Obtém as dimensões da imagem
    vet = lines[1].split() #for i in range(2)) => widht = int(vet[0]) = 60 and height = int(vet[1]]) = 8
    width, height = [int(vet[i]) for i in range(len(vet))]
    print(width, height)

    # initialize the matrix of pixels
    pixels = [[0] * width for _ in range(height)]

    # Run all image lines and fill the matrix of pixels
    for i in range(height):
        row = lines[i + 2].strip()
        for j in range(width):
            pixels[i][j] = int(row[j])

    return pixels



# Imprime a matrix de pixels
def showMat(pixels):
    for i in range(len(pixels)): #height, rowls
        for j in range(len(pixels[i])): #widht, cols
            print(pixels[i][j],end='')
        print()
    print()



def dilata(pixels):
    height, width = len(pixels), len(pixels[0]) #height = 8, width = 60
    dil = [[0] * width for _ in range(height)]

    for i in range(height):
        for j in range(width):          
            max = pixels[i][j]
            for x in range(-1,2):
                for y in range(-1,2):
                    viz_i = i + x
                    viz_j = j + y
                    if 0 <= viz_i < height and 0 <= viz_j < width:
                        if pixels[viz_i][viz_j] > max:
                            max = pixels[viz_i][viz_j]
            dil[i][j] = max
    return dil


# Exemplo de uso
filename = input("write file's name: ")
pixels = leiaImagemPBM(filename)
showMat(pixels)
showMat(dilata(pixels))



