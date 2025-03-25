#include <SD.h>

// Conexiones:
// ---------------------------
// MOSI       ->  Pin 51
// MISO       ->  Pin 50
// SCK        ->  Pin 52
// CS         ->  Pin 53 
// VCC        ->  5V
// GND        ->  GND

File myFile;

void setup() {
    Serial.begin(9600);
    delay(1000);  // Esperar un poco

    Serial.println("Iniciando SD...");
    if (!SD.begin(53)) {  
        Serial.println("Error: No se pudo inicializar la tarjeta SD.");
        return;
    }
    Serial.println("SD inicializada correctamente.");

    Serial.println("Creando archivo...");
    myFile = SD.open("test1.txt", FILE_WRITE);

    if (!myFile) {
        Serial.println("Error: No se pudo crear test.txt. Intentando eliminar y recrear...");
        SD.remove("test.txt");  // Eliminar el archivo si ya existe
        myFile = SD.open("test.txt", FILE_WRITE);  // Volver a abrir
    }

    if (myFile) {
        Serial.println("Archivo creado con éxito. Escribiendo...");
        myFile.println("Esto es una prueba de guardado en la SD.");
        myFile.close();
        Serial.println("Datos guardados correctamente.");
    } else {
        Serial.println("Error: No se pudo abrir test.txt.");
    }
}

void loop() {
    // No hace falta nada aquí
}

