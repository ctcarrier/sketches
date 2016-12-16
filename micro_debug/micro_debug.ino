/*********************

Example code for the Adafruit RGB Character LCD Shield and Library

This code displays text on the shield, and also reads the buttons on the keypad.
When a button is pressed, the backlight changes color.

**********************/

// include the library code:
#include <Wire.h>
#include "DHT.h"
#include <SoftwareSerial.h>
#include <XBee.h>

#define DHTPIN 5     // what digital pin we're connected to

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

#define rxPin 2
#define txPin 3

int RXLED = 17;  // The RX LED has a defined Arduino pin

#define MEASUREMENT_DELAY 10000

// The shield uses the I2C SCL and SDA pins. On classic Arduinos
// this is Analog 4 and 5 so you can't use those for analogRead() anymore
// However, you can connect other I2C sensors to the I2C bus and share
// the I2C bus.
//Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

//SoftwareSerial mySerial =  SoftwareSerial(rxPin, txPin);
XBee xbee = XBee();
TxStatusResponse txStatus = TxStatusResponse();

DHT dht(DHTPIN, DHTTYPE);

int numDigits(int num) {
  int res = 0;
  if (num < 10) res = 1;
  else if (num >= 10 && num < 100) res = 2;
  else if (num >= 100) res = 3;

  return res;
}

Tx64Request getApiRequest(uint8_t *payload, int paloadLength) {
  //uint64_t addr64 = 0x0013A2004147284D;
  //XBeeAddress64 addr = XBeeAddress64(addr64); 
  //XBeeAddress64 addr = XBeeAddress64(0x0013A2004147284D);
  XBeeAddress64 addr = XBeeAddress64(0x00000000, 0x0000ffff);
  uint8_t payload2[] = { 0, 0 };
  return Tx64Request(addr, payload, paloadLength);
}

void setup() {
  pinMode(RXLED, OUTPUT);  // Set RX LED as an output
  pinMode(13, OUTPUT);
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  //mySerial.begin(9600);
  Serial1.begin(9600);
  xbee.setSerial(Serial1);
  // Debugging output
  Serial.begin(9600);    
}

uint8_t i=0;
void loop() {    
  delay(MEASUREMENT_DELAY);
  long start = millis();
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!\n");
    digitalWrite(RXLED, LOW);   // set the LED on
    //mySerial.println("Failed to read from DHT sensor!");
  }
  else {
    digitalWrite(RXLED, HIGH);   // set the LED on
    // Compute heat index in Fahrenheit (the default)
    float hif = dht.computeHeatIndex(f, h);
    // Compute heat index in Celsius (isFahreheit = false)
    float hic = dht.computeHeatIndex(t, h, false);
    
    int intH = (int) h;
    int intF = (int) f;  
    char charF[numDigits(f) +1];
    char charH[numDigits(h) +1];
    sprintf(charF, "%d", intF);  
    sprintf(charH, "%d", intH);  
    int totalMessageLength = numDigits(h) + numDigits(f) + 1; //total length with delimiter
    char totalMessage[totalMessageLength];
    strcpy(totalMessage, charH);
    strcat(totalMessage, ",");
    strcat(totalMessage, charF);
    strcat(totalMessage, '\0');
    uint8_t *finalMessage = (uint8_t*)totalMessage;    

    Serial.println(totalMessage);
    Tx64Request req = getApiRequest(finalMessage, totalMessageLength);    
    for (int i = 0; i < totalMessageLength; i++) {
      Serial.print((char)finalMessage[i]);  
    }    
    Serial.print("\n");
    xbee.send(req);
    delay(500);
    digitalWrite(RXLED, HIGH);   // set the LED on
    delay(500);
    digitalWrite(RXLED, LOW);   // set the LED on
    delay(500);
    digitalWrite(RXLED, HIGH);   // set the LED on
    // after sending a tx request, we expect a status response
    // wait up to 5 seconds for the status response
    if (xbee.readPacket(5000)) {
        // got a response!
  
        // should be a znet tx status              
      if (xbee.getResponse().getApiId() == TX_STATUS_RESPONSE) {
         xbee.getResponse().getTxStatusResponse(txStatus);
        
         // get the delivery status, the fifth byte
           if (txStatus.getStatus() == SUCCESS) {
              // success.  time to celebrate
              Serial.println("Success\n");
           } else {
              // the remote XBee did not receive our packet. is it powered on?
              Serial.println("Failure");
           }
        }      
    } else if (xbee.getResponse().isError()) {
      Serial.println("Error reading packet.  Error code: ");  
      Serial.println(xbee.getResponse().getErrorCode());    
    } else {
      Serial.println("No response");
    }       
  }
  long runtime = millis() - start;    
}
