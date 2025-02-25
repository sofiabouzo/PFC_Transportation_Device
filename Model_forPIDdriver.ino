// --- LIBRERÍAS ---
#include <Arduino.h>

// --- CONFIGURACIÓN ---
// Pines
const int pinSensor = A0;    // Entrada del termistor NTC
const int pinMosfet = 9;     // Salida PWM al MOSFET IRF520

// Constantes del controlador PI
float Kp = 1.2;  // Ganancia proporcional
float Ki = 0.6;  // Ganancia integral

// Setpoint de temperatura (°C) Es la DESEADA
float setpoint = 4.0; 

// Variables del control PI
float integral = 0.0; //La suma acumulada del error, usada en la parte integral del controlador.
float lastError = 0.0; //No se usa en PI!! serviría si quisiéramos agregar la parte derivativa para un PID
float dt = 30.0;  // Tiempo de muestreo! cada cuanto ACTUALIZAMOS EL CONTROLADOR

// --- FUNCIÓN PARA LEER TEMPERATURA --- CAMBIAR !! POR EL NUESTRO
float leerTemperatura() {
    int raw = analogRead(pinSensor);  // Leer ADC
    float resistencia = (1023.0 / raw - 1) * 10000.0; // Conversión ADC a resistencia
    float temperatura = 1.0 / (0.001129148 + (0.000234125 * log(resistencia)) + (0.0000000876741 * pow(log(resistencia), 3))) - 273.15;
    return temperatura;
} // CAMBIAR !! POR EL NUESTRO

// --- FUNCIÓN PARA CONTROL PI ---
int controlPI(float tempActual) {
    float error = setpoint - tempActual;
    integral += error * dt;
    
    float salida = (Kp * error) + (Ki * integral);
    salida = constrain(salida, 0, 255); // Limitar PWM a rango válido
    return int(salida);
}

// --- LOOP PRINCIPAL ---
void setup() {
    pinMode(pinSensor, INPUT);
    pinMode(pinMosfet, OUTPUT);
    Serial.begin(9600);
}

void loop() {
    float temperatura = leerTemperatura();
    int pwmSalida = controlPI(temperatura);

    analogWrite(pinMosfet, pwmSalida);

    // Mostrar valores por Serial
    Serial.print("Temp: ");
    Serial.print(temperatura);
    Serial.print(" °C | PWM: ");
    Serial.println(pwmSalida);

    delay(1000);  // Muestreo cada 1s
}
