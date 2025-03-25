import serial
import csv
import time
from datetime import datetime


PORT = "COM9" 
#PORT= '/dev/cu.usbmodem1101'


BAUD_RATE = 9600

# Abrir conexión con Arduino
arduino = serial.Serial(PORT, BAUD_RATE)
time.sleep(2)  # Esperar a que Arduino inicie
fecha_hora = datetime.now().strftime("%Y-%m-%d_%H-%M") 
filename = f"C:\\Users\\VALENTINA\\PFC_Transportation_Device\\registros\\datos_{fecha_hora}_prefrio_cargaTempAmb.csv"
#filename = f"/Users/sofia/Desktop/PFC_Transportation_Device/registros/datos_{fecha_hora}.csv"


# Abrir archivo CSV para guardar datos
with open(filename, "w", newline="") as file:
    writer = csv.writer(file)
    writer.writerow(["Tiempo", "Resistencia", "Temperatura"])  # Encabezados

    print(f"Guardando datos en {filename}...")
    i=0
    while True:
        try:
            dato = arduino.readline().decode().strip()  # Leer dato de Arduino
            
            if dato and not dato.startswith("Tiempo"):  # Omitir la línea de encabezado
                valores = dato.split(",")  # Separar los valores
                
                print(f"Guardado: {valores}")
                if i==0 or i==30:
                    writer.writerow(valores)  # Guardar en CSV
                    if i==30:
                        i= 0
                i+=1
        except KeyboardInterrupt:
            print("\nFinalizando...")
            break

arduino.close()