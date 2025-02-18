import math
import serial
import time
import numpy as np

# Configuración
CONFIG_THERMISTOR_RESISTOR = 9850  # Resistencia en serie con el termistor (ohms)
serialArduino=None
# Puerto serie
serialArduino = serial.Serial(port= "/dev/tty.usbmodem1101" , baudrate=9600, bytesize=serial.EIGHTBITS, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, timeout=0.5)
time.sleep(1)
if serialArduino:
    print('conexion exitosa')

# Función para calcular la resistencia del termistor
def thermistor_get_resistance(adcval):
    """Calcula la resistencia del termistor NTC a partir del valor del ADC."""
    return CONFIG_THERMISTOR_RESISTOR * ((1023.0 / adcval) - 1)

# Función para calcular la temperatura en °C
def thermistor_get_temperature(resistance):
    """Calcula la temperatura en grados Celsius usando la ecuación de Steinhart-Hart."""
    # Coeficientes de Steinhart-Hart
    A = 0.00013851124897641025
    B = 0.0004137990275744092
    C = -0.000000681439906738466
    
    # Logaritmo natural de la resistencia
    temp = math.log(resistance)
    
    # Ecuación de Steinhart-Hart
    temp = 1 / (A + (B * temp) + (C * temp * temp * temp))
    
    # Convertir de Kelvin a Celsius
    return temp - 273.15

# Función principal
signalI = []
def main():
    serialArduino.write(b's')
    valSI = serialArduino.read(size=202)
    for i in range(0, len(valSI) - 1, 2):
        val2 = ord(valSI[i:i+1])
        val3 = ord(valSI[i+1:i+2])
        val4 = val2*256+val3
        signalI.append(val4)
        R= thermistor_get_resistance(val4)
        temp= np.round(thermistor_get_temperature(R),2)
        print(temp)
    

if __name__ == "__main__":
    main()
