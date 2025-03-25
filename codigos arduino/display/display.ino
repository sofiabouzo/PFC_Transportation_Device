#include <LiquidCrystal.h>
#include <Arduino.h>
#include <math.h>
#include <SD.h>


// Display connections: 5V Y GROUND!!
// ---------------------------
// D4         ->  Pin 28 (rojo) 
// D5         ->  Pin 26 (azul)
// D6         ->  Pin 24 (naranja)
// D7         ->  Pin 22 (verde)
// RS         ->  Pin 25 (blanco)
// E          ->  Pin 23 (amarillo)

const int rs = 25, en = 23, d4 = 28, d5 = 26, d6 = 24, d7 = 22;
#define btn A0
#define CONFIG_THERMISTOR_ADC_PIN A1
#define CONFIG_THERMISTOR_RESISTOR 9890l //resistencia en serie con termistor
#define SD_CS_PIN 53
bool editingDate = false;  // flag to start editing the date
int position = 0;  // selected field:  0: day, 1: month
int day = 1, month = 1; // starting date
const int year=25;

// time management
unsigned long currentTime; // time since the Arduino was initialized 
unsigned long lastSample = 0;
const long freq = 3000; // 30 s


String selectedDate;

// Variables para temperatura
float ultimaTemperatura = 0.0; // Nueva variable global
uint32_t ultimaResistencia = 0;
bool recordingState= true;

// buttons dictionary:
#define btnRIGHT  0 // analog read: 2
#define btnUP     1 // analog read: 142
#define btnDOWN   2 // analog read: 336
#define btnLEFT   3 // analog read: 503
#define btnSELECT 4 // analog read: 723
#define btnNONE   5 // analog read: 1018 & 1019

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  
  lcd.begin(16, 2); // LCD number of columns and rows
  Serial.begin(9600); // intialize series comunication

  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("Fallo al inicializar SD");
    lcd.setCursor(0, 0);
    lcd.print("SD Error");
    while (1); 
  }

  welcomeMessage();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ingrese fecha:");
  lcd.setCursor(0, 1);
  lcd.print(">01/ 01/25"); 

  editingDate = true; // Start editing date
}

// --------------- WELCOME MESSAGE ----------------
void welcomeMessage() {
  String message = "Bienvenido!! Iniciando...";
  lcd.clear();
  
  for (int i = 0; i < message.length(); i++) {
    lcd.setCursor(0, 0);
    lcd.print(message.substring(i));
    delay(400);  // Velocidad de scroll
  }
  delay(1000);
}

// --------------- READ KEYPAD ----------------
int read_lcd_buttons() {
  int val = analogRead(btn); 
  if (val > 1000) return btnNONE; 
  if (val < 50) return btnRIGHT;  
  if (val > 50 && val < 200) return btnUP; 
  if (val > 200 && val < 400) return btnDOWN; 
  if (val > 400 && val < 600) return btnLEFT; 
  if (val > 600 && val < 800) return btnSELECT;
  return btnNONE;
}

// --------------- EDIT POSITION ----------------
void editDate(int key) {
  if (!editingDate) return;
 if (key == btnRIGHT) {
    position = (position + 1) % 2;
  }
  if (key == btnLEFT) {
    position = (position - 1 + 2) % 2;
  }
  // Confirm date with SELECT
  if (key == btnSELECT) {
    selectedDate = String(day < 10 ? "0" : "") + String(day) + "-" + String(month < 10 ? "0" : "") + String(month) + "-25";
    editingDate = false; // exits edition mode
  }
}

// --------------- UPDATE DATE ----------------
void updateDate(int button) {
  switch (position) {
    case 0: // Day
      if (button == btnUP) {
        if (day < 31) day++;
        else day = 1; 
      }
      if (button == btnDOWN) {
        if (day > 1) day--;
        else day = 31;
      }
      break;

    case 1: // Month
      if (button == btnUP) {
        if (month < 12) month++;
        else month = 1; 
      }
      if (button == btnDOWN) {
        if (month > 1) month--;
        else month = 12;
      }
      break;
  }
}

// --------------- UPDATE DISPLAY ----------------
void displayData() {
  static bool lastState = true; // flag to only update display if there are changes 
  if (editingDate) {
    lcd.setCursor(0, 1);
    
    // day
    if (position == 0) lcd.print(">"); else lcd.print(" ");
    lcd.print(day < 10 ? "0" : "");
    lcd.print(day);
    lcd.print("/");
    
    // month
    if (position == 1) lcd.print(">"); else lcd.print(" ");
    lcd.print(month < 10 ? "0" : "");
    lcd.print(month);
    lcd.print("/");
    
    // year 
    lcd.print("25");
  } 
  else if(recordingState){
    lcd.setCursor(0, 0);
    lcd.print(selectedDate);
    lcd.print("         ");
    lcd.setCursor(0, 1);
    lcd.print("Temp: ");
    lcd.print(ultimaTemperatura, 2); 
    lcd.print(" C   ");
    lcd.print("        ");
  }
  else { 
    if (lastState != recordingState) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("FINALIZADO");
      lastState = recordingState;
 
    }
  }
}
// --------------- READ NTC ----------------

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


// --------------- SAVE DATA AS CSV ----------------
void ntc_data(){
  uint32_t ultimaResistencia = thermistor_get_resistance(analogRead(CONFIG_THERMISTOR_ADC_PIN));
  ultimaTemperatura = thermistor_get_temperature(ultimaResistencia); // actualizamos la variable global
    
   
}


// --------------- SAVE DATA AS CSV ----------------
void saveSD(unsigned long t) {
  String fileName = selectedDate + String(".csv");
  File archivo = SD.open(fileName, FILE_WRITE);
  if (archivo) {
    archivo.print(t / 1000);
    archivo.print(",");
    archivo.println(ultimaTemperatura, 2);
    archivo.flush();
    archivo.close();
  } else {
    lcd.clear();
    lcd.print("Error al abrir archivo");
  }
}



// --------------- SET DATE LCD ----------------
void set_date_lcd(){
  int key = read_lcd_buttons();

  if (key != btnNONE) {
    if (editingDate) {
      editDate(key);       // Handle navigation and Select
      updateDate(key);     // Update date based on button input
      lcd.setCursor(0, 1); // Reset cursor to start of second line
      lcd.print("        "); // Clear the line (8 spaces for DD/MM/YY)
      displayData();       // Show the current date
    } else if (key == btnSELECT && recordingState ) {
      stopRecording(); // Detener con SELECT después de confirmar fecha
    }
    delay(200);           // Basic debounce to prevent multiple triggers
  }
}

// --------------- STOP RECORDING ----------------
void stopRecording() {
  // Último guardado para asegurar que no se pierdan datos
  saveSD(currentTime);
  recordingState = false; // Detener mediciones y guardado
  displayData(); // Mostrar mensaje de detenido
}
  
void loop() {
  set_date_lcd();

  currentTime= millis();
  if (!editingDate && recordingState) {
    // Medir temperatura
    if (lastSample == 0 || (currentTime - lastSample >= freq) ) {
      lastSample = currentTime;
      ntc_data();
      saveSD(currentTime);
      displayData();
      }
  }
  else if (!editingDate && !recordingState) {
    displayData();
  }
}











