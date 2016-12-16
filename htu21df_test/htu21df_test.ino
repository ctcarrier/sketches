/*********************

Example code for the Adafruit RGB Character LCD Shield and Library

This code displays text on the shield, and also reads the buttons on the keypad.
When a button is pressed, the backlight changes color.

**********************/

// include the library code:
#include <Wire.h>
#include "Adafruit_HTU21DF.h"
#include <SoftwareSerial.h>
#include <XBee.h>

#define DHTPIN 5     // what digital pin we're connected to

#define RXPIN 10
#define TXPIN 11

int RXLED = 17;  // The RX LED has a defined Arduino pin

#define MEASUREMENT_DELAY 1000

// The shield uses the I2C SCL and SDA pins. On classic Arduinos
// this is Analog 4 and 5 so you can't use those for analogRead() anymore
// However, you can connect other I2C sensors to the I2C bus and share
// the I2C bus.
//Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

Adafruit_HTU21DF htu = Adafruit_HTU21DF();
SoftwareSerial mySerial(RXPIN, TXPIN);

int convertToFahrenheit(float c) {
  return c * 1.8 + 32;
}

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  Serial.println("Setup");
 htu.begin();
}

uint8_t i=0;
void loop() {    
  Serial.println("Looping");  
  long start = millis();
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = htu.readHumidity();
  float t = htu.readTemperature();
  // Read temperature as Celsius (the default)
  Serial.println(h);
  Serial.println(t);
}
