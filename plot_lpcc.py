#!  /usr/bin/python3

import matplotlib.pyplot as plt
import numpy as np
import soundfile as sf


fdatos = open('lpcc_2_3.txt', 'r')
x_datos = []                #  Creamos una lista para la primera columna
y_datos = []                #  Creamos una lista para la segunda columna
lineas = fdatos.readlines() # Leemos el fichero línea a línea
for linea in lineas:
    x, y = linea.split()     # Se separa cada línea en dos columnas
    x_datos.append(float(x)) # Añado el elemento x a la lista x_datos
    y_datos.append(float(y)) # Añado el elemento y a la lista y_datos

fdatos.close()


plt.figure(figsize=(10,7))
plt.plot(x_datos,y_datos,'o',markersize=2)
plt.show()
