/* 
This is a test sketch for the Adafruit assembled Motor Shield for Arduino v2
It won't work with v1.x motor shields! Only for the v2's with built in PWM
control

For use with the Adafruit Motor Shield v2 
---->  http://www.adafruit.com/products/1438
*/


#include <Wire.h>
#include "LPD8806.h"
#include "SPI.h"

#define rxPin 5
#define flashPin 6

int dataPin  = 10;
int clockPin = 11;
int nLEDs = 100;

LPD8806 strip = LPD8806(nLEDs, dataPin, clockPin);

void setup() {
  Serial.begin(9600);           // set up Serial library at 9600 bps  

  strip.begin();
  strip.show();
}

void loop() {    
  for (int i=0; i < strip.numPixels(); i++) {
    Serial.println("Setting color");
    //strip.setPixelColor(i, strip.Color(127, 127, 127));
    strip.setPixelColor(i, 0);
  }
  strip.show(); 
}
