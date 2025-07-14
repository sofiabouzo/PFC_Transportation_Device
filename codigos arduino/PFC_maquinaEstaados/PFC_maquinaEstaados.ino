#include <LiquidCrystal.h>
#include <Arduino.h>
#include <math.h>
#include <SD.h>
#include <PID_v1.h>

//definimos para el timer
#define _clk ((float) 16000000)     // Clock de Arduino Mega [Hz]
#define _regPcalI (0<<CS32 | 1<<CS31 | 1<<CS30)  // Prescaler = 64
#define _pcalI 64

#define BATTERY_LEVEL A0
#define btn A1
#define CONFIG_THERMISTOR_ADC_PIN A2
#define CONFIG_THERMISTOR_RESISTOR 9890l //resistencia en serie con termistor
#define SD_CS_PIN 53

#define GREEN_LED 9
#define YLLW_LED 10
#define RED_LED 11
#define DOOR_PIN 8
#define BUZZER_PIN 12   // Pin del MODULADOR del buzzer (HIGH se apaga)

#define LED_25_PIN 27
#define LED_50_PIN 29
#define LED_75_PIN 31
#define LED_100_PIN 33




// Display connections: 5V Y GROUND!!
// ---------------------------
// D4         ->  Pin 28 (rojo) 
// D5         ->  Pin 26 (azul)
// D6         ->  Pin 24 (naranja)
// D7         ->  Pin 22 (verde)
// RS         ->  Pin 25 (blanco)
// E          ->  Pin 23 (amarillo)


// Conexiones:
// ---------------------------
// MOSI       ->  Pin 51
// MOSO       ->  Pin 50
// SCK        ->  Pin 52
// CS         ->  Pin 53 
// VCC        ->  5V
// GND        ->  GND

typedef enum{
  INIT,
  EDITING_DATE,
  RECORDING,
  IDLE
} Status;

typedef enum {
  TEMP_OK,
  TEMP_HIGH,
  TEMP_DANGER
} TempStatus;

// buttons dictionary:
typedef enum{
  RIGHT,
  UP,
  DOWN,
  LEFT,
  SELECT,
  NONE
} Button;

Status currentState = INIT;
const float fs = 1000;           // Frecuencia deseada del ISR [Hz] 30
const int rs = 25, en = 23, d4 = 28, d5 = 26, d6 = 24, d7 = 22;

TempStatus tempState = TEMP_DANGER;

volatile uint32_t counter = 0;
volatile bool flagSampleTime = false;
bool buzzerMuted = true;
bool noSD= true;
bool initialized= false;

unsigned long openTime = 0;
bool buzzerOn = false;

int position = 0;  // represents the selected field:  0: day, 1: month
int day = 1, month = 1; // starting date
const int year=25;
int pinPWM = 4; // pwm pinout 

const float R1= 1800000;
const float R2= 2200000;

// time management
unsigned long currentTime; // time since the recording was initialized
unsigned long lastSample = 0;
unsigned long previousTime=0;
unsigned long startRecordingTime = 0;
const long freq = 3000; // 3 s
unsigned long lastSave = 0;
int status= 1; // to seee if temp is above or below setpoint

String selectedDate;

// Variables para temperatura
float ultimaTemperatura = 0.0; // Nueva variable global
uint32_t ultimaResistencia = 0;

//Constants for PID
const float Kp=5;
const float Ki=0.05;
float setPoint = 4.0; 
const float eiLim = 25000; 
float temp = 0;
float error = 0;
float ep = 0;
float ei = 0;
float output_pid = 128;
unsigned long lastTime = 0;
unsigned long sampleInterval = 3000; // [ms] son 3s

PID pid((double *) &ultimaTemperatura, (double *) &output_pid, (double *) &setPoint, Kp, Ki, 0, REVERSE);

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YLLW_LED, OUTPUT);
  pinMode(LED_100_PIN, OUTPUT);
  pinMode(LED_75_PIN, OUTPUT);
  pinMode(LED_50_PIN, OUTPUT);
  pinMode(LED_25_PIN, OUTPUT);

  pinMode(DOOR_PIN, INPUT_PULLUP); 
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW); // Apagado al inicio

  pinMode(pinPWM, OUTPUT);

  pid.SetOutputLimits(0, 255);
  pid.SetSampleTime(sampleInterval);
  pid.SetMode(AUTOMATIC);

  lcd.begin(16, 2); // LCD number of columns and rows

  cli(); // Deshabilito interrupciones globales

  // Configuro Timer3 en modo CTC
  TCCR3A = 0;
  TCCR3B = 0;
  TCCR3B |= (1 << WGM32) | _regPcalI; // Modo CTC + prescaler = 64
  TCNT3 = 0;  // Reseteo contador
  OCR3A = (uint16_t)((_clk / (fs * _pcalI)) - 1); // Valor de comparación
  TIMSK3 |= (1 << OCIE3A);  // Habilito interrupción por comparación A

  sei(); // Habilito interrupciones globales

  Serial.begin(9600); // intialize series comunication

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
Button readLCDButton() {
  int val = analogRead(btn);
  if (val > 1000) return NONE;
  else if (val < 50) return RIGHT;
  else if (val < 200) return UP;
  else if (val < 400) return DOWN;
  else if (val < 600) return LEFT;
  else if (val < 800) return SELECT;
  else return NONE;
}

// --------------- EDIT POSITION ----------------
void editDate(Button key) {
  switch (key){
    case RIGHT:
      position = (position + 1) % 2;
      break;

    case LEFT:
      position = (position - 1 + 2) % 2;
      break;

    case SELECT: // Confirm date with SELECT
      selectedDate = String(day < 10 ? "0" : "") + String(day) + "-" + String(month < 10 ? "0" : "") + String(month) + "-25";
      startRecordingTime = millis();
      currentState= RECORDING; // exits edition mode
      break;
      
    default:
      break;
  }
}

// --------------- UPDATE DATE ----------------
void updateDate(Button button) {
  switch (position) {
    case 0: // Day

      switch (button) {
        case UP:
          if (day < 31) day++;
          else day = 1; 
          break;
        case DOWN:
          day = (day > 1) ? day - 1 : 31;
          if (day > 1) day--;
          else day = 31;
          break;

        default:
          break;
      }
      break;

    case 1: // Month
    switch (button) {
        case UP:
          if (month < 12) month++;
          else month = 1; 
          break;

        case DOWN: 
          if (month > 1) month--;
          else month = 12;
          break;

        default:
          break;
      }
  break;
  }
}

// --------------- UPDATE DISPLAY ----------------
void displayData() {
  switch (currentState) {
    case EDITING_DATE:
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

      break;

    case RECORDING:
      lcd.setCursor(0, 0);
      lcd.print(selectedDate);
      lcd.print("         ");
      lcd.setCursor(0, 1);
      lcd.print("Temp: ");
      lcd.print(ultimaTemperatura, 2); 
      lcd.print(" C   ");
      lcd.print("        ");
      //Serial.println(ultimaTemperatura); // borrar esto al final!!

      break; 

    case IDLE:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("FINALIZADO");
      delay(1000);
      break;
  }
}

// --------------- READ NTC ----------------

int32_t thermistor_get_resistance(uint16_t adcval){
  float Voc= adcval*5/1023.0;
  return(CONFIG_THERMISTOR_RESISTOR *(Voc/(5-Voc)));
}

float thermistor_get_temperature(int32_t resistance){
  float temp = log(resistance);
  temp = 1 / (0.0011487729471781924 + 0.0002291945054659473 * temp + 1.1834801498089633e-7 * temp * temp * temp);
  return (temp - 273.15);
}

// --------------- GET TEMPERATURE ----------------
void ntc_data(){
  uint32_t ultimaResistencia = thermistor_get_resistance(analogRead(CONFIG_THERMISTOR_ADC_PIN));
  ultimaTemperatura = thermistor_get_temperature(ultimaResistencia); // actualizamos la variable global
  if (ultimaTemperatura >= 14.0) tempState= TEMP_DANGER; 
  else if (ultimaTemperatura > 8) tempState= TEMP_HIGH;
  else tempState= TEMP_OK; // if (ultimaTemperatura >2) 
}

// --------------- MANAGE LED ON/OFF ----------------
void ledOutput(){
  switch (tempState){
    case TEMP_OK: // green on
      digitalWrite(RED_LED, LOW);
      digitalWrite(YLLW_LED, LOW);
      digitalWrite(GREEN_LED, HIGH);
      break;

    case TEMP_DANGER: // red on
      digitalWrite(RED_LED, HIGH);
      digitalWrite(YLLW_LED, LOW);
      digitalWrite(GREEN_LED, LOW);
      break;

    case TEMP_HIGH: // yellow on
      digitalWrite(RED_LED, LOW);
      digitalWrite(YLLW_LED, HIGH);
      digitalWrite(GREEN_LED, LOW);
      break;
  }
}

// --------------- SAVE DATA AS CSV ----------------
void saveSD(unsigned long t) {
  String fileName = selectedDate + String(".csv");
  File archivo = SD.open(fileName, FILE_WRITE);
  
  if (archivo) {
    archivo.print(t / 1000);
    archivo.print(",");
    archivo.print(ultimaTemperatura, 2);
    archivo.print(",");
    archivo.println(255 - output_pid, 2); 
    archivo.flush();
    archivo.close();
  } else {
    lcd.clear();
    lcd.print("Error de archivo");
  }
}

// --------------- RING BUZZER ----------------
void alarm(){
  int doorState = digitalRead(DOOR_PIN);
  if (doorState == HIGH) {  // HIGH -> puerta abierta
    openTime+=1;
    if (openTime>= 5000){
      digitalWrite(BUZZER_PIN, HIGH); // Activa el buzzer si quedo abierta por mas de 5s
      buzzerOn = true;
    }
  }
  else{
    digitalWrite(BUZZER_PIN, LOW);
    buzzerOn = false;
    openTime=0;
  }

}
// max battery --> 9V
// empty battery --> 6V (no longer works as desired)
// del divisor resistivo, R1= 1.8Mohm R2= 2.2Mohm
void batteryIndicator(){
  int batPin = analogRead(BATTERY_LEVEL);
  float Vadc= batPin * 5.0 / 1023.0;
  float Vbat = Vadc * (R1 + R2) / R2;

  if (Vbat >= 8.9) {
    digitalWrite(LED_100_PIN, HIGH);
    digitalWrite(LED_75_PIN, HIGH);
    digitalWrite(LED_50_PIN, HIGH);
    digitalWrite(LED_25_PIN, HIGH);
  }
  else if (Vbat >= 8.25) {
    digitalWrite(LED_100_PIN, LOW);
    digitalWrite(LED_75_PIN, HIGH);
    digitalWrite(LED_50_PIN, HIGH);
    digitalWrite(LED_25_PIN, HIGH);
  }
  else if (Vbat >= 7.5) {
    digitalWrite(LED_100_PIN, LOW);
    digitalWrite(LED_75_PIN, LOW);
    digitalWrite(LED_50_PIN, HIGH);
    digitalWrite(LED_25_PIN, HIGH);
  }
  else if (Vbat >= 6.75) {
    digitalWrite(LED_100_PIN, LOW);
    digitalWrite(LED_75_PIN, LOW);
    digitalWrite(LED_50_PIN, LOW);
    digitalWrite(LED_25_PIN, HIGH);
  }
  else {
  digitalWrite(LED_100_PIN, LOW);
  digitalWrite(LED_75_PIN, LOW);
  digitalWrite(LED_50_PIN, LOW);

  // 🔁 Parpadeo del LED de 25%
  static unsigned long lastBlink = 0;
  static bool ledState = false;
  if (millis() - lastBlink >= 500) {  // 500 ms ON, 500 ms OFF
    lastBlink = millis();
    ledState = !ledState;
    digitalWrite(LED_25_PIN, ledState);
  }
}
}

// ISR runs every 1ms
ISR(TIMER3_COMPA_vect) {
    if(counter > 0) counter--;
    else {
      counter = 3000;
      flagSampleTime = true; // marca la ocurrencia del evento
    }
}


void loop() {
  Button button = readLCDButton();

  switch (currentState) {
    case INIT:
      if (!SD.begin(SD_CS_PIN)) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("SD Error");
        noSD = true;
      } 
      else {
        lcd.clear();
        welcomeMessage();

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Ingrese fecha:");
        lcd.setCursor(0, 1);
        lcd.print(">01/ 01/25");
        delay(1000); // for debouncing

        currentState = EDITING_DATE;
      }
  
  break;

    case EDITING_DATE:
      editDate(button);       // Handle navigation and Select
      updateDate(button);     // Update date based on button input
      //lcd.setCursor(0, 1); // Reset cursor to start of second line
      //lcd.print("        "); // Clear the line (8 spaces for DD/MM/YY)
      displayData();       // Show the current date
      delay(200); // for debouncing
      
      break;
    
    case RECORDING:
      alarm();
      if (flagSampleTime) {
        // de la ISR que se ejecuta cada Ts = 1/fs segundos sale esta flag
        flagSampleTime = false; //la consumo
        ntc_data();
        currentTime = millis() - startRecordingTime;
        pid.Compute();                // Calcula nueva salida PWM
        analogWrite(pinPWM, 255-output_pid);

        saveSD(currentTime);

        displayData();
        ledOutput();
        batteryIndicator();

      }


      if (button == SELECT) currentState= IDLE;
      break;
    
    case IDLE:
      analogWrite(pinPWM, 255);
      currentTime = millis() - startRecordingTime;
      saveSD(currentTime);
      displayData(); // Shows the stopping message
      break;

    default:
      break;
  }
}

