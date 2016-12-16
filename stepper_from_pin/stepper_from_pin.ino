/* 
This is a test sketch for the Adafruit assembled Motor Shield for Arduino v2
It won't work with v1.x motor shields! Only for the v2's with built in PWM
control

For use with the Adafruit Motor Shield v2 
---->  http://www.adafruit.com/products/1438
*/


#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"
#include "LPD8806.h"
#include "SPI.h"

#define rxPin 5
#define flashPin 6

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
// Or, create it with a different I2C address (say for stacking)
// Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61); 

// Connect a stepper motor with 200 steps per revolution (1.8 degree)
// to motor port #2 (M3 and M4)
Adafruit_StepperMotor *myMotor = AFMS.getStepper(200, 2);
int run = 0;
int haveStepped = 0;

int dataPin  = 10;
int clockPin = 11;
int nLEDs = 100;

LPD8806 strip = LPD8806(nLEDs, dataPin, clockPin);

void setup() {
  Serial.begin(9600);           // set up Serial library at 9600 bps
  Serial.println("Stepper test!");

  pinMode(rxPin, INPUT); 
  pinMode(flashPin, INPUT); 

  AFMS.begin();  // create with the default frequency 1.6KHz
  //AFMS.begin(1000);  // OR with a different frequency, say 1KHz
  
  myMotor->setSpeed(30);  // 10 rpm   

  strip.begin();
  strip.show();
}

void loop() {  
  if (digitalRead(rxPin) == HIGH && haveStepped == 0){
    myMotor->step(10, FORWARD, SINGLE);    
    haveStepped = 1;
  }  
  if (digitalRead(rxPin) == LOW && haveStepped == 1){
    haveStepped = 0;
  }
  if (digitalRead(flashPin) == HIGH){
    for (int i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(127, 127, 127));
    }
    strip.show();
  }  
  if (digitalRead(flashPin) == LOW){
    for (int i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, 0);
    }
    strip.show();
  }  
}
