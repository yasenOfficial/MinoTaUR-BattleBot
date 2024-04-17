#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#define rellayPin 10
#define rellayPinMagnet A1

#define motor1Pin 2
#define direction_pin1 3
#define stop_pin1 4

#define motor2Pin 5
#define direction_pin2 6
#define stop_pin2 9

RF24 radio(7, 8); // CE, CSN

Adafruit_MPU6050 mpu;

const byte address[6] = "00001";

void setup() {
  pinMode(direction_pin1, OUTPUT);
  pinMode(stop_pin1, OUTPUT);
  pinMode(direction_pin2, OUTPUT);
  pinMode(stop_pin2, OUTPUT);
  pinMode(rellayPin, OUTPUT);
  pinMode(rellayPinMagnet, OUTPUT);

  Serial.begin(9600);

  radio.begin();
  radio.openReadingPipe(1, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();

  if (!mpu.begin()) {
		Serial.println("Failed to find MPU6050 chip");
		while (1) {
		  delay(10);
		}
	}

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);

	// set gyro range to +- 500 deg/s
	mpu.setGyroRange(MPU6050_RANGE_500_DEG);

	// set filter bandwidth to 21 Hz
	mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
}

// Structure to store received data
struct {
  int joy1X;
  int joy1Y;
  int joy2X;
  int joy2Y;
  bool rellayController;
  bool rellayControllerPinMagnet;
}receivedData;

struct {
  int batteryLevel;
  float speed;
  bool isFlipped;
}dataToSend;

int motor1Dir = 0;
int motor2Dir = 0;

void loop() {
  if (radio.available()) {
    radio.read(&receivedData, sizeof(receivedData));
    
    // Map the joystick X values from the range 0-1023 to the range 0-255
    // int mappedJoy1X = map(receivedData.joy1X, 0, 1023, 0, 255);
    // int mappedJoy2X = map(receivedData.joy2X, 0, 1023, 0, 255);
    Serial.println(receivedData.joy1X);    
    // Control Motor 1 based on joystick 1 input
    if (receivedData.joy1X <= 200) {
      if(motor1Dir < 1){
        digitalWrite(stop_pin1, LOW);
        motor1Dir = 1;
      }else{
        int mappedJoy1X = map(receivedData.joy1X, 320, 0, 0, 255);
        digitalWrite(stop_pin1, HIGH);
        digitalWrite(direction_pin1, LOW);
        analogWrite(motor1Pin, mappedJoy1X);
        }
    } else if (receivedData.joy1X >= 820 ) {
      if(motor1Dir > -1){
        digitalWrite(stop_pin1, LOW);
        motor1Dir = -1;
      }else{
        int mappedJoy1X = map(receivedData.joy1X, 700, 1023, 0, 255);
        digitalWrite(stop_pin1, HIGH);
        digitalWrite(direction_pin1, HIGH);
        analogWrite(motor1Pin, mappedJoy1X);
        }
    } else {
        digitalWrite(stop_pin1, LOW);
    }
    
    // Control Motor 2 based on joystick 2 input
    if (receivedData.joy2X <= 200) {
      if(motor2Dir < 1){
        digitalWrite(stop_pin2, LOW);
        motor2Dir = 1;
      }else{
        int mappedJoy2X = map(receivedData.joy2X, 320, 0, 0, 255);
        digitalWrite(stop_pin2, HIGH);
        digitalWrite(direction_pin2, LOW);
        analogWrite(motor2Pin, mappedJoy2X);
        }
    } else if (receivedData.joy2X >= 820) {
        if(motor2Dir > -1){
          digitalWrite(stop_pin2, LOW);
          motor2Dir = -1;
        }else{
        int mappedJoy2X = map(receivedData.joy2X, 700, 1023, 0, 255);
        digitalWrite(stop_pin2, HIGH);
        digitalWrite(direction_pin2, HIGH);
        analogWrite(motor2Pin, mappedJoy2X);
        }
    } else {
        digitalWrite(stop_pin2, LOW);
    }
    
    // Control relay based on relay controller input
    digitalWrite(rellayPin, receivedData.rellayController);
    digitalWrite(rellayPinMagnet, receivedData.rellayControllerPinMagnet);
    
}

  sensors_event_t a, g, t;
	mpu.getEvent(&a, &g, &t);

  dataToSend.batteryLevel = 100;
  dataToSend.isFlipped = g.gyro.y;
  dataToSend.speed = a.acceleration.x;

  delay(10);

  radio.stopListening();

  radio.openWritingPipe(address);
  radio.write(&dataToSend, sizeof(dataToSend));
  radio.openReadingPipe(1, address);
  radio.startListening();
}
