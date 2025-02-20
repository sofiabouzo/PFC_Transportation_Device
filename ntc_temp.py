import serial
import csv
import time

PORT = "COM6" 


BAUD_RATE = 9600

# Abrir conexión con Arduino
arduino = serial.Serial(PORT, BAUD_RATE)
time.sleep(2)  # Esperar a que Arduino inicie

filename = "datos_20.02_concarga.csv"

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
                if i==29:
                    writer.writerow(valores)  # Guardar en CSV
                    i=-1
                i+=1
        except KeyboardInterrupt:
            print("\nFinalizando...")
            break

arduino.close()