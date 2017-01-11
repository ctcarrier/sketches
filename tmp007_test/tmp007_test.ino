#include <Wire.h>
#include "Adafruit_HTU21DF.h"
#include "Adafruit_TMP007.h"
#include <SoftwareSerial.h>

#define TMP_007_I2C_ADDR 0x45

Adafruit_TMP007 tmp007 = Adafruit_TMP007(TMP_007_I2C_ADDR);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  if (! tmp007.begin()) {
    Serial.println("No TMP007 found");    
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  float objTemp = tmp007.readObjTempC();
  float dieTemp = tmp007.readDieTempC();
  Serial.print(objTemp); Serial.println(" for object");
  Serial.print(dieTemp); Serial.println(" for die");

  delay(2000);
}
