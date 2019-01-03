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
#include <Adafruit_BMP085_U.h>

#define DHTPIN 5     // what digital pin we're connected to

int RXLED = 17;  // The RX LED has a defined Arduino pin

#define MEASUREMENT_DELAY 600000

#define RXPIN 5
#define TXPIN 6

#define TMP_007_I2C_ADDR 0x45

// The shield uses the I2C SCL and SDA pins. On classic Arduinos
// this is Analog 4 and 5 so you can't use those for analogRead() anymore
// However, you can connect other I2C sensors to the I2C bus and share
// the I2C bus.
//Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

XBee xbee = XBee();
ZBTxStatusResponse txStatus = ZBTxStatusResponse();
SoftwareSerial mySerial(RXPIN, TXPIN);

//Adafruit_HTU21DF htu = Adafruit_HTU21DF();
Adafruit_TMP007 tmp007 = Adafruit_TMP007(TMP_007_I2C_ADDR);
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);

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

  if(!bmp.begin())
  {
    /* There was a problem detecting the BMP085 ... check your connections */
    Serial.print("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!");    
  }
  
  /* Display some basic information on this sensor */
  displaySensorDetails();
}

void displaySensorDetails(void)
{
  sensor_t sensor;
  bmp.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" hPa");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" hPa");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" hPa");  
  Serial.println("------------------------------------");
  Serial.println("");  
}

sensors_event_t getTslData() {
  /* Get a new sensor event */ 
  sensors_event_t event;
  tsl.getEvent(&event);
 
  return event;
}

sensors_event_t getBpmData() {
  sensors_event_t event;
  bmp.getEvent(&event);

  return event;
}

int numDigits(long num) {
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
  return ZBTxRequest(addr, payload, strlen(payload));
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
  
//  if (! htu.begin()) {
//    Serial.println("No HTU found");    
//  }
  if (! tmp007.begin()) {
    Serial.println("No TMP007 found");    
  }
  configureSensor();
}

void sendToXbee(float objTemp, float dieTemp, sensors_event_t luxEvent, sensors_event_t pressureEvent, boolean xbeeEnabled) {

  if (xbeeEnabled) {
    sendBmpRequest(pressureEvent);       
    sendTslRequest(luxEvent);
    sendTmpRequest(objTemp, dieTemp);
  }  
}

ZBTxRequest sendBmpRequest(sensors_event_t pressureEvent) {
  float pressure = pressureEvent.pressure;
  unsigned long intPressure = (unsigned long) pressure * 100;
  float c;
  bmp.getTemperature(&c);  
  unsigned long intC = (unsigned long) (c * 100);    
  
  char charC[numDigits(intC) + 1];
  char charPressure[numDigits(intPressure) + 1];  
  ltoa(intC, charC, 10);  
  ltoa(intPressure, charPressure,10); 
  char *sensorName = "bmp180,";
  
  int totalMessageLength = numDigits(intPressure) + numDigits(intC) + strlen(sensorName) + 2; //total length with 2 delimiters and null
  char totalMessage[totalMessageLength] = {0}; 
  strcat(totalMessage, sensorName);  
  strcat(totalMessage, charPressure);
  strcat(totalMessage, ",");
  strcat(totalMessage, charC);
  strcat(totalMessage, '\0');
  uint8_t *finalMessage = (uint8_t*)totalMessage;        

  Serial.println(totalMessage);
  ZBTxRequest req = getApiRequest(finalMessage);  

  for (int i = 0; i < sizeof(totalMessage); i++) {
    Serial.print((char)totalMessage[i]);  
  }    
  Serial.print("\n");
  
  handleXbeeMessage(req);
}

ZBTxRequest sendTmpRequest(float objTemp, float dieTemp) {  
  unsigned long intObjTemp = (unsigned long) objTemp * 100;    
  unsigned long intDieTemp = (unsigned long) (dieTemp * 100);    
  
  char charObjTemp[numDigits(intObjTemp) + 1];
  char charDieTemp[numDigits(intDieTemp) + 1];  
  ltoa(intObjTemp, charObjTemp, 10);  
  ltoa(intDieTemp, charDieTemp,10); 
  char *sensorName = "tmp007,";
  
  int totalMessageLength = numDigits(intObjTemp) + numDigits(intDieTemp) + strlen(sensorName) + 2; //total length with 2 delimiters and null
  char totalMessage[totalMessageLength] = {0}; 
  strcat(totalMessage, sensorName);  
  strcat(totalMessage, charObjTemp);
  strcat(totalMessage, ",");
  strcat(totalMessage, charDieTemp);
  strcat(totalMessage, '\0');
  uint8_t *finalMessage = (uint8_t*)totalMessage;        

  Serial.println(totalMessage);
  ZBTxRequest req = getApiRequest(finalMessage);  

  for (int i = 0; i < sizeof(totalMessage); i++) {
    Serial.print((char)totalMessage[i]);  
  }    
  Serial.print("\n");
  
  handleXbeeMessage(req);
}

ZBTxRequest sendTslRequest(sensors_event_t luxEvent) {
  float light = luxEvent.light;
  unsigned long intLight = (unsigned long) light * 100;  
  
  char charLight[numDigits(intLight) + 1];    
  ltoa(intLight, charLight,10); 
  char *sensorName = "tsl2561,";
  
  int totalMessageLength = numDigits(intLight) + strlen(sensorName) + 1; //total length with null
  char totalMessage[totalMessageLength] = {0}; 
  strcat(totalMessage, sensorName);    
  strcat(totalMessage, charLight);
  strcat(totalMessage, '\0');
  uint8_t *finalMessage = (uint8_t*)totalMessage;        

  Serial.println(totalMessage);
  ZBTxRequest req = getApiRequest(finalMessage);  

  for (int i = 0; i < sizeof(totalMessage); i++) {
    Serial.print((char)totalMessage[i]);  
  }    
  Serial.print("\n");
  
  handleXbeeMessage(req);
}

void handleXbeeMessage(ZBTxRequest req) {
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

void validateReadings(float objTemp, float dieTemp, sensors_event_t luxEvent, sensors_event_t pressureEvent) {
  // Check if any reads failed and exit early (to try again).
  if (isnan(objTemp) || isnan(dieTemp)) {
    Serial.println("Failed to read from TMP007 sensor!\n");
    digitalWrite(RXLED, LOW);   // set the LED on
    //mySerial.println("Failed to read from DHT sensor!");
  }
  else {
    digitalWrite(RXLED, HIGH);   // set the LED on    
            
  }

  if (!luxEvent.light) {
    Serial.println("Failed to read from TSL2561 sensor!");
  }
  if (!pressureEvent.pressure) {
    Serial.println("Failed to read from BMP100 sensor!");
  }
}

void logReadings(float objTemp, float dieTemp, sensors_event_t luxEvent, sensors_event_t pressureEvent) {  
  Serial.print("Obj Temp: ");
  Serial.println(convertToFahrenheit(objTemp));
  Serial.print("Die Temp: ");
  Serial.println(convertToFahrenheit(dieTemp));

  /* Display the results (light is measured in lux) */
  if (luxEvent.light) {
    Serial.print(luxEvent.light); Serial.println(" lux");
  }
  else {
    /* If event.light = 0 lux the sensor is probably saturated
       and no reliable data could be generated! */
    Serial.println("Sensor overload");
  }

  if (pressureEvent.pressure)
  {
    /* Display atmospheric pressue in hPa */
    Serial.print("Pressure:    ");
    Serial.print(pressureEvent.pressure);
    Serial.println(" hPa");
    
    /* Calculating altitude with reasonable accuracy requires pressure    *
     * sea level pressure for your position at the moment the data is     *
     * converted, as well as the ambient temperature in degress           *
     * celcius.  If you don't have these values, a 'generic' value of     *
     * 1013.25 hPa can be used (defined as SENSORS_PRESSURE_SEALEVELHPA   *
     * in sensors.h), but this isn't ideal and will give variable         *
     * results from one day to the next.                                  *
     *                                                                    *
     * You can usually find the current SLP value by looking at weather   *
     * websites or from environmental information centers near any major  *
     * airport.                                                           *
     *                                                                    *
     * For example, for Paris, France you can check the current mean      *
     * pressure and sea level at: http://bit.ly/16Au8ol                   */
     
    /* First we get the current temperature from the BMP085 */
    float temperature;
    bmp.getTemperature(&temperature);
    Serial.print("Temperature: ");
    Serial.print(convertToFahrenheit(temperature));
    Serial.println(" C");

    /* Then convert the atmospheric pressure, and SLP to altitude         */
    /* Update this next line with the current SLP for better results      */
    float seaLevelPressure = 1027.43;
    Serial.print("Altitude:    "); 
    Serial.print(bmp.pressureToAltitude(seaLevelPressure,
                                        pressureEvent.pressure)); 
    Serial.println(" m");
    Serial.println("");
  }
  else
  {
    Serial.println("Sensor error");
  }
}

uint8_t i=0;
long runtime = 0;
void loop() {    
  delay(MEASUREMENT_DELAY - runtime);
  long start = millis();  

  float objTemp = tmp007.readObjTempC();
  float dieTemp = tmp007.readDieTempC();

  sensors_event_t luxEvent = getTslData();
  sensors_event_t pressureEvent = getBpmData();

  logReadings(objTemp, dieTemp, luxEvent, pressureEvent);
  validateReadings(objTemp, dieTemp, luxEvent, pressureEvent);
  sendToXbee(objTemp, dieTemp, luxEvent, pressureEvent, true);
  
  runtime = millis() - start;    
}
