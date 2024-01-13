#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

RF24 radio(7, 8); // CE, CSN

const byte address[6] = "00001";

const int joyXPin = A0;  // X-axis pin
const int joyYPin = A1;  // Y-axis pin
const int joySWPin = 2;  // Joystick switch pin

LiquidCrystal_I2C lcd(0x27,  16, 2);

void setup() {
 Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(1, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
  
  pinMode(joySWPin, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();
}

struct {
  int joyX;
  int joyY;
  int joySW;
}dataToSend;

struct {
  int batteryLevel;
  float speed;
  bool isFlipped;
}receivedData;

void loop() {
    Serial.println("-----");

  if (radio.available()) {
    while (radio.available()){
      radio.read( &receivedData, sizeof(receivedData) );
    }
    Serial.println("Recieve: ");
    Serial.print("Package:");
    Serial.print(receivedData.batteryLevel);
    Serial.print("\n");
    Serial.println(receivedData.speed);
    Serial.println(receivedData.isFlipped);
    Serial.print("\n");
  }

  // Read analog values from X and Y axes
  int joyXValue = analogRead(joyXPin);
  int joyYValue = analogRead(joyYPin);

  // Read the state of the joystick switch
  int joySWState = digitalRead(joySWPin);

  dataToSend.joyX = joyXValue;
  dataToSend.joyY = joyYValue;
  dataToSend.joySW = joySWState;

  delay(10);

  radio.stopListening();
  radio.openWritingPipe(address);
  radio.write(&dataToSend, sizeof(dataToSend));
  radio.openReadingPipe(1, address);
  radio.startListening();

  lcd.setCursor(0,0);
  lcd.print("Battery: ");
  lcd.print(receivedData.batteryLevel);
  lcd.print("%");
  lcd.setCursor(0,1);
  lcd.print("Sped: ");
  lcd.print(receivedData.speed);
}