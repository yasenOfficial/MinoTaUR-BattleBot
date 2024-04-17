#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

RF24 radio(7, 8); // CE, CSN

const byte address[6] = "00001";

const int joy1XPin = A0;  // X-axis pin
const int joy1YPin = A1;  // Y-axis pin
const int joy2XPin = A2;  // X-axis pin
const int joy2YPin = A3;  // Y-axis pin
const int rellayControllerPin = 3;
const int rellayControllerPinMagnet = 2;

LiquidCrystal_I2C lcd(0x27,  16, 2);

void setup() {
  pinMode(rellayControllerPin, INPUT_PULLUP);
  pinMode(rellayControllerPinMagnet, INPUT_PULLUP);
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(1, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();

  lcd.init();
  lcd.backlight();
}

struct {
  int joy1X;
  int joy1Y;
  int joy2X;
  int joy2Y;
  bool rellayController;
  bool rellayControllerPinMagnet;
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
  int joy1XValue = analogRead(joy1XPin);
  int joy1YValue = analogRead(joy1YPin);

  int joy2XValue = analogRead(joy2XPin);
  int joy2YValue = analogRead(joy2YPin);
  //Serial.println(joyXValue);

  dataToSend.joy1X = joy1XValue;
  dataToSend.joy1Y = joy1YValue;
  dataToSend.joy2X = joy2XValue;
  dataToSend.joy2Y = joy2YValue;
  dataToSend.rellayController = !digitalRead(rellayControllerPin); 
  dataToSend.rellayControllerPinMagnet = !digitalRead(rellayControllerPinMagnet); 

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