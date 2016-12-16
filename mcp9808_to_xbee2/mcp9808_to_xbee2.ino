/*********************

Example code for the Adafruit RGB Character LCD Shield and Library

This code displays text on the shield, and also reads the buttons on the keypad.
When a button is pressed, the backlight changes color.

**********************/

// include the library code:
#include <Wire.h>
#include "Adafruit_HTU21DF.h"
#include "Adafruit_TMP007.h"
#include <SoftwareSerial.h>
#include <XBee.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>

#define DHTPIN 5     // what digital pin we're connected to

int RXLED = 17;  // The RX LED has a defined Arduino pin

#define MEASUREMENT_DELAY 10000

#define RXPIN 10
#define TXPIN 11

#define TMP_007_I2C_ADDR 0x45

// The shield uses the I2C SCL and SDA pins. On classic Arduinos
// this is Analog 4 and 5 so you can't use those for analogRead() anymore
// However, you can connect other I2C sensors to the I2C bus and share
// the I2C bus.
//Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

XBee xbee = XBee();
ZBTxStatusResponse txStatus = ZBTxStatusResponse();
SoftwareSerial mySerial(RXPIN, TXPIN);

Adafruit_HTU21DF htu = Adafruit_HTU21DF();
Adafruit_TMP007 tmp007 = Adafruit_TMP007(TMP_007_I2C_ADDR);
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

void configureSensor(void)
{
  /* You can also manually set the gain or enable auto-gain support */
  // tsl.setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */
  // tsl.setGain(TSL2561_GAIN_16X);     /* 16x gain ... use in low light to boost sensitivity */
  tsl.enableAutoRange(true);            /* Auto-gain ... switches automatically between 1x and 16x */
  
  /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */

  /* Update these values depending on what you've set above! */  
  Serial.println("------------------------------------");
  Serial.print  ("Gain:         "); Serial.println("Auto");
  Serial.print  ("Timing:       "); Serial.println("13 ms");
  Serial.println("------------------------------------");

  if(!tsl.begin())
  {
    /* There was a problem detecting the TSL2561 ... check your connections */
    Serial.print("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!");    
  }
}

sensors_event_t getTslData() {
  /* Get a new sensor event */ 
  sensors_event_t event;
  tsl.getEvent(&event);
 
  return event;
}

int numDigits(int num) {
  int res = 0;
  if (num < 10) res = 1;
  else if (num >= 10000) res = 5;
  else if (num >= 1000) res = 4; 
  else if (num >= 100) res = 3;
  else if (num >= 10 && num < 100) res = 2;    
  
  return res;  
}

int convertToFahrenheit(float c) {
  return c * 1.8 + 32;
}

ZBTxRequest getApiRequest(uint8_t *payload) {
  //uint64_t addr64 = 0x0013A2004147284D;
  XBeeAddress64 addr = XBeeAddress64(0x00000000, 0x00000000);
  uint8_t payload2[] = { 0, 0 };
  return ZBTxRequest(addr, payload, sizeof(payload));
}

void setup() {
  Serial.println("Setup");
  pinMode(RXLED, OUTPUT);  // Set RX LED as an output
  pinMode(13, OUTPUT);  
  //xbee.setSerial(Serial1);
  
  // Debugging output
  Serial.begin(9600); 
  mySerial.begin(9600);   

  xbee.setSerial(mySerial);
  
  if (! htu.begin()) {
    Serial.println("No HTU found");    
  }
  if (! tmp007.begin()) {
    Serial.println("No TMP007 found");    
  }
  configureSensor();
}

void sendToXbee(float h, float c, float f, float objTemp, float dieTemp, sensors_event_t event, boolean xbeeEnabled) {
  int intH = (int) (h * 100);
  int intF = (int) (f * 100);  
  
  char charF[numDigits(intF) +1];
  char charH[numDigits(intH) +1];
  sprintf(charF, "%d", intF);  
  sprintf(charH, "%d", intH);  
  int totalMessageLength = numDigits(intH) + numDigits(intF) + 1; //total length with delimiter
  char totalMessage[totalMessageLength];
  strcpy(totalMessage, charH);
  strcat(totalMessage, ",");
  strcat(totalMessage, charF);
  strcat(totalMessage, '\0');
  uint8_t *finalMessage = (uint8_t*)totalMessage;        

  Serial.println(totalMessage);
  ZBTxRequest req = getApiRequest(finalMessage);    
  for (int i = 0; i < sizeof(totalMessage); i++) {
    Serial.print((char)totalMessage[i]);  
  }    
  Serial.print("\n");

  if (xbeeEnabled) {
    xbee.send(req);
  
    // after sending a tx request, we expect a status response
    // wait up to 5 seconds for the status response
    if (xbee.readPacket(5000)) {
        // got a response!
  
        // should be a znet tx status              
      if (xbee.getResponse().getApiId() == TX_STATUS_RESPONSE) {
         xbee.getResponse().getTxStatusResponse(txStatus);
        
         // get the delivery status, the fifth byte
           if (txStatus.getDeliveryStatus() == SUCCESS) {
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
}

void logReadings(float h, float c, float f, float objTemp, float dieTemp, sensors_event_t event) {
  Serial.print("Humidity: ");
  Serial.println(h);
  Serial.print("F: ");
  Serial.println(f);
  Serial.print("C: ");
  Serial.println(c);
  Serial.print("Obj Temp: ");
  Serial.println(convertToFahrenheit(objTemp));
  Serial.print("Die Temp: ");
  Serial.println(convertToFahrenheit(dieTemp));

  /* Display the results (light is measured in lux) */
  if (event.light) {
    Serial.print(event.light); Serial.println(" lux");
  }
  else {
    /* If event.light = 0 lux the sensor is probably saturated
       and no reliable data could be generated! */
    Serial.println("Sensor overload");
  }
}

uint8_t i=0;
void loop() {    
  delay(MEASUREMENT_DELAY);
  long start = millis();
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = htu.readHumidity();
  // Read temperature as Celsius (the default)
  float c = htu.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = convertToFahrenheit(c);

  float objTemp = tmp007.readObjTempC();
  float dieTemp = tmp007.readDieTempC();

  sensors_event_t event = getTslData();

    logReadings(h, c, f, objTemp, dieTemp, event);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(c) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!\n");
    digitalWrite(RXLED, LOW);   // set the LED on
    //mySerial.println("Failed to read from DHT sensor!");
  }
  else {
    digitalWrite(RXLED, HIGH);   // set the LED on
    
    sendToXbee(h, c, f, objTemp, dieTemp, event, false);        
  }
  long runtime = millis() - start;    
}
