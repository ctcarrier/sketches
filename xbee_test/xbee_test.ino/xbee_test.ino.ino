/*********************

Example code for the Adafruit RGB Character LCD Shield and Library

This code displays text on the shield, and also reads the buttons on the keypad.
When a button is pressed, the backlight changes color.

**********************/

// include the library code:
#include <Wire.h>
#include <SoftwareSerial.h>

#define rxPin 2
#define txPin 3

SoftwareSerial mySerial =  SoftwareSerial(rxPin,txPin);

char hexDigit(unsigned n)
{
    if (n < 10) {
        return n + '0';
    } else {
        return (n - 10) + 'A';
    }
}

void charToHex(char c, char hex[2])
{
    hex[0] = hexDigit(c / 0x10);
    hex[1] = hexDigit(c % 0x10);    
}

void setup() {
  pinMode(13, OUTPUT);
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  mySerial.begin(9600);
  // Debugging output
  Serial.begin(9600);
  
}

uint8_t i=0;
void loop() {    
  while (mySerial.available()) {    
    char hex[2];
    int in = (int)mySerial.read();
    //charToHex(in, hex);
    //Serial.print(hex);    
    Serial.print(in);
    Serial.print(" ");
  }
  delay(1000);
}
