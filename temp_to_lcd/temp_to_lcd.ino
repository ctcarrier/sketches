/*********************

Example code for the Adafruit RGB Character LCD Shield and Library

This code displays text on the shield, and also reads the buttons on the keypad.
When a button is pressed, the backlight changes color.

**********************/

// include the library code:
#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
#include "DHT.h"

#define DHTPIN 7     // what digital pin we're connected to

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321


// The shield uses the I2C SCL and SDA pins. On classic Arduinos
// this is Analog 4 and 5 so you can't use those for analogRead() anymore
// However, you can connect other I2C sensors to the I2C bus and share
// the I2C bus.
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  // Debugging output
  Serial.begin(9600);
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  
}

uint8_t i=0;
void loop() {  
  delay(2000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    Serial.println(h);
    Serial.println(t);
    Serial.println(f);
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Humidity: ");
  lcd.print(h);
  lcd.setCursor(0,1);  
  lcd.print(t);
  lcd.print("C ");
  lcd.print(f);
  lcd.print("F");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");
  //uint8_t buttons = lcd.readButtons();

//  if (buttons) {
//    lcd.clear();
//    lcd.setCursor(0,0);
//    if (buttons & BUTTON_UP) {
//      lcd.print("UP ");
//    }
//    if (buttons & BUTTON_DOWN) {
//      lcd.print("DOWN ");
//    }
//    if (buttons & BUTTON_LEFT) {
//      lcd.print("LEFT ");
//    }
//    if (buttons & BUTTON_RIGHT) {
//      lcd.print("RIGHT ");
//    }
//    if (buttons & BUTTON_SELECT) {
//      lcd.print("SELECT ");
//    }
//  }
}
