/*********************

Example code for the Adafruit RGB Character LCD Shield and Library

This code displays text on the shield, and also reads the buttons on the keypad.
When a button is pressed, the backlight changes color.

**********************/

// include the library code:
#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
#include <SoftwareSerial.h>


// The shield uses the I2C SCL and SDA pins. On classic Arduinos
// this is Analog 4 and 5 so you can't use those for analogRead() anymore
// However, you can connect other I2C sensors to the I2C bus and share
// the I2C bus.
//Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

SoftwareSerial mySerial =  SoftwareSerial(2,3);
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

void setup() {
  pinMode(13, OUTPUT);
  mySerial.begin(9600);
  // Debugging output
  Serial.begin(9600);
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  
}

uint8_t i=0;
void loop() {  
  if (mySerial.available()) {
    lcd.clear();
    lcd.setCursor(0,0);
  }
  while (mySerial.available()) {
    char in =(char)mySerial.read();
    Serial.print(in);
    lcd.print(in);
  }
  delay(1000);
}
