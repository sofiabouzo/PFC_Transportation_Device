#include <Arduino.h>
#include <math.h>

#define CONFIG_THERMISTOR_ADC_PIN A0
#define CONFIG_THERMISTOR_RESISTOR 9890l //resistencia en serie con termistor

int32_t thermistor_get_resistance(uint16_t adcval)
{
  return (CONFIG_THERMISTOR_RESISTOR * ((1023.0 / adcval) - 1));
}

float thermistor_get_temperature(int32_t resistance)
{
  float temp = log(resistance);
  temp = 1 / (0.0011487729471781924 + 0.0002291945054659473 * temp + 1.1834801498089633e-7 * temp * temp * temp);
  return (temp - 273.15);
}

void setup()
{
  Serial.begin(9600);
  while (!Serial);

  Serial.println(F("Tiempo,Resistencia,Temperatura"));  // Encabezados CSV
}

void loop()
{
  uint32_t resistencia = thermistor_get_resistance(analogRead(CONFIG_THERMISTOR_ADC_PIN));
  float temperatura = thermistor_get_temperature(resistencia);

  // Obtener el tiempo desde que Arduino arrancó (en milisegundos)
  unsigned long tiempo = millis();

  // Enviar datos en formato CSV
  Serial.print(tiempo/1000); //pasamos a segundos
  Serial.print(",");
  Serial.print(resistencia);
  Serial.print(",");
  Serial.println(temperatura, 2);

  delay(1000);  // Tomar muestras cada 1 segundo
}
