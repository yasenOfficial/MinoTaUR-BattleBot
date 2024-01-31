#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#define direction_pin 6
#define stop_pin 5

#define rellayPin 2

RF24 radio(7, 8); // CE, CSN

Adafruit_MPU6050 mpu;

const byte address[6] = "00001";

void setup() {
  pinMode(direction_pin, OUTPUT);
  pinMode(stop_pin, OUTPUT);
  pinMode(rellayPin, OUTPUT);

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
  int joyX;
  int joyY;
  int joySW;

}receivedData;

struct {
  int batteryLevel;
  float speed;
  bool isFlipped;
}dataToSend;

void loop() {
  digitalWrite(rellayPin, HIGH);
  if (radio.available()) {
    radio.read(&receivedData, sizeof(receivedData));
    // Map the joystick X value from the range 0-1023 to the range 0-255
    if(receivedData.joyX >= 640){
      int mappedJoyX = map(receivedData.joyX, 508, 1023, 0, 255);
      digitalWrite(direction_pin, LOW);
      digitalWrite(stop_pin, HIGH);
      analogWrite(3, mappedJoyX);
      Serial.println("Greater than 508");
    }else if(receivedData.joyX <= 630){
      int mappedJoyX = map(receivedData.joyX, 508, 0, 0, 255);
      digitalWrite(direction_pin, HIGH);
      digitalWrite(stop_pin, HIGH);
      analogWrite(3, mappedJoyX);
      Serial.println("Less than 508");
    }else{
      digitalWrite(stop_pin, LOW);
    }


    Serial.println("-----");
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

  delay(100);

}
