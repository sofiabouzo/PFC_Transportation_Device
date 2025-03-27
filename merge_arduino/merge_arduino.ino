int buzzer = 9;  // Pin del buzzer
int estado; 
int buttonPin = 10;
int buttonRead; 

void setup() {
  Serial.begin(9600); // Iniciar comunicación serie
  pinMode(buttonPin, INPUT);
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, HIGH);

  
  
}

void loop() {
  Serial.println();
  //Serial.println("¿qué temperatura hay?");
  buttonRead = digitalRead(buttonPin);
  Serial.println(buttonRead);
  
  while (Serial.available() == 0) { 
    
  }  

  estado = Serial.parseInt();
  buttonRead = digitalRead(buttonPin);
  Serial.println(buttonRead);
  
  /*if (estado > 10){
    digitalWrite(buzzer, LOW);
    delay(500);
    digitalWrite(buzzer, HIGH); 
  }*/
  while (estado > 10){
    digitalWrite(buzzer, LOW);
    delay(500);
    digitalWrite(buzzer, HIGH); 
    delay(500);


    buttonRead = digitalRead(buttonPin);
    
    if (buttonRead == 0) {
      Serial.println("✅ Alarma desactivada.");
      digitalWrite(buzzer, HIGH); // Apaga el buzzer
      break;
    }
  }
} 

      


/*void loop(){
  digitalWrite(buzzer,HIGH);
  delay(500);
  digitalWrite(buzzer,LOW);
  
  delay(500);
}
int buzzPin = 8;
int potVal;
int potPin = A5;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(buzzPin, OUTPUT);
  pinMode(potPin, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:

  potVal = analogRead(potPin);
  Serial.println(potVal);

  while (potVal>1000){
    digitalWrite(buzzPin, HIGH);
    potVal = analogRead(potPin);
    Serial.println(potVal);
  }
  digitalWrite(buzzPin, LOW);

}
*/
