import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

df = pd.read_csv("registros/datos_18.02_sincarga.csv")
x = df["Tiempo"].values[:89]
y = df["Temperatura"].values[:89]

x= np.insert(x,0,0)
y= np.insert(y,0, 22.0)

d2y = np.gradient(np.gradient(y, x), x)

indices = np.where(np.diff(np.sign(d2y)))[0]
x_inflexion = x[indices]
y_inflexion= y[indices]



# Punto de tangencia
idx= 0
x_tan = x_inflexion[idx]
y_tan = y_inflexion[idx]

dy_dx = np.gradient(y, x) 

m = dy_dx[indices[idx]]




# Definir la ecuación de la recta tangente: y = m(x - x0) + y0
x_line = np.linspace(min(x),max(x))  # Rango para la recta
y_line = m * (x_line - x_tan) + y_tan  # Ecuación de la tangente

raiz= -y_tan/m + x_tan
#T del proceso hasta que alcanzamos la asintota en este caso 3.13
y_T= 3.13 #temp asintota
x_T= (y_T-y_tan)/m + x_tan
T_lat = 30
T_proceso = x_T - T_lat
delta_temp= y.max()-y_T
tension= 11

k0= (tension*T_proceso)/(delta_temp*T_lat) #V/ºC
print("k0", k0, "delta", delta_temp)

print("pendiente", m, "b", y_tan, "a", x_tan)
print(y_line)
print(x_T, y_T)

plt.plot(x,y)
plt.plot(x_line, y_line, color="green", linestyle="solid", label="Tangente en x={:.2f}".format(x_tan))
plt.ylim(0, max(y))
plt.scatter(x_inflexion,y[indices])
plt.scatter(raiz,0, color='black')
plt.plot(x_T, y_T, 'ro') 
plt.axhline(y_T, color='red')

plt.show()

tabla = pd.DataFrame(index=range(3), columns=['Controlador', 'Kp', 'Ki', 'Kd'])
# Ki = Kp/Ti --> Ti = L/0.3 en PI 
# Kd = Kp * Td  --> En PID Ti=2L y Td = 0.5 L

tabla.loc[0] = ['P', k0 , '-', '-']
tabla.loc[1] = ['PI', 0.9*k0 , 0.27*k0/T_lat, '-']
tabla.loc[2] = ['PID', 1.2*k0, 0.6*k0/T_lat, 0.6*k0]

print(tabla)