#include <Wire.h>
#include <SHT1x.h>
#include <Adafruit_AM2315.h>

#include <SoftwareSerial.h>
#include <XBee.h>

#define dataPin 10
#define clockPin 11

#define RXPIN 5
#define TXPIN 6

int RXLED = 17;

SHT1x sht1x(dataPin, clockPin);
Adafruit_AM2315 am2315;

XBee xbee = XBee();
ZBTxStatusResponse txStatus = ZBTxStatusResponse();
SoftwareSerial mySerial(RXPIN, TXPIN);

boolean xbeeEnabled = true;
boolean loggingEnabled = true;

void setup() {
  mySerial.begin(9600);
  Serial.begin(9600);
  Serial.println("AM2315 Test!");
  pinMode(RXLED, OUTPUT);  // Set RX LED as an output

  xbee.setSerial(mySerial);

  while (! am2315.begin()) {
     Serial.println("Sensor not found, check wiring & pullups!");
     delay(5000);
  }
}

void loop() {
  Serial.println("\n\n\n\n");
  getAndProcessReadings();  

  delay(600000);
}

void getAndProcessReadings() {
  Serial.println("Getting and proceseing");
  float shtTemp = sht1x.readTemperatureC();
  float shtHumidity = sht1x.readHumidity();

  float amHumidity = am2315.readHumidity();
  float amTemp = am2315.readTemperature();

  handleData(shtHumidity, shtTemp, "SHT01", "Error reading from SHT01 sensor.\n");
  handleData(amHumidity, amTemp, "AM2315", "Error reading from AM2315 sensor.\n");
}

void handleData(float humidity, float temp, char* sensorName, char* message) {
  if (validateReadings(humidity, temp, message)) {
    logReadings(humidity, temp, sensorName);
    sendHumidityAndTempRequest(humidity, temp, sensorName);
  }
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

ZBTxRequest sendHumidityAndTempRequest(float humidity, float temp, char* sensorName) { 
  if (xbeeEnabled) { 
    unsigned long intHumidity = (unsigned long) humidity * 100;  
    unsigned long intC = (unsigned long) (temp * 100);    
    
    char charC[numDigits(intC) + 1];
    char charHumidity[numDigits(intHumidity) + 1];  
    ltoa(intC, charC, 10);  
    ltoa(intHumidity, charHumidity, 10); 
    
    int totalMessageLength = numDigits(intHumidity) + numDigits(intC) + strlen(sensorName) + 2; //total length with 2 delimiters and null
    char totalMessage[totalMessageLength] = {0}; 
    strcat(totalMessage, sensorName);  
    strcat(totalMessage, ",");
    strcat(totalMessage, charHumidity);
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

boolean validateReadings(float humidity, float temp, char* message) {
  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(temp)) {
    Serial.println(message);
    digitalWrite(RXLED, LOW);   // set the LED on    
    return false;
  }
  digitalWrite(RXLED, HIGH);   // set the LED on                
  return true;
}

void logReadings(float humidity, float temp, char* sensorName) {  
  if (loggingEnabled) {
    Serial.print(sensorName); Serial.println(" results:");
    Serial.println("-------------------------------------");
    Serial.print("Temp ");
    Serial.println(temp);
    Serial.print("Humidity ");
    Serial.println(humidity);
    Serial.println("\n");
  }
}

ZBTxRequest getApiRequest(uint8_t *payload) {
  //uint64_t addr64 = 0x0013A2004147284D;
  XBeeAddress64 addr = XBeeAddress64(0x00000000, 0x00000000);
  uint8_t payload2[] = { 0, 0 };
  return ZBTxRequest(addr, payload, strlen(payload));
}
