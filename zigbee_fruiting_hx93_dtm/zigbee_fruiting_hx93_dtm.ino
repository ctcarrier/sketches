#include <Wire.h>

#include <SoftwareSerial.h>
#include <XBee.h>

#define dataPin 10
#define clockPin 11

#define RXPIN 5
#define TXPIN 6

#define HX93_TEMP A0
#define HX93_RH A4

#define DTM_TEMP A1
#define DTM_RH A2
#define DTM_GAS A3

int RXLED = 17;

XBee xbee = XBee();
ZBTxStatusResponse txStatus = ZBTxStatusResponse();
SoftwareSerial mySerial(RXPIN, TXPIN);

boolean xbeeEnabled = true;
boolean loggingEnabled = true;

void setup() {
  mySerial.begin(9600);
  Serial.begin(9600);
  Serial.println("Starting sensors");
  pinMode(RXLED, OUTPUT);  // Set RX LED as an output
  pinMode(A0, INPUT);
  pinMode(A4, INPUT);

  xbee.setSerial(mySerial);
}

void loop() {
  Serial.println("\n\n\n\n");
  getAndProcessReadings();
  getAndProcessDtmReadings();

  delay(10000);
}

void getAndProcessReadings() {
  Serial.println("Getting and proceseing");  

  int tempResist = analogRead(HX93_TEMP);
  float tempVoltage = tempResist * (5.0 / 1023.0);

  int humidityResist = analogRead(HX93_RH);
  float humidityVoltage = humidityResist * (5.0 / 1023.0);

  float finalHumidity = humidityVoltage * 100.0;
  float finalF =  (tempVoltage/.005848) - 4;

  handleData(finalHumidity, finalF, "HX93", "Error reading from HX93 sensor.\n");  
}

void getAndProcessDtmReadings() {
  Serial.println("Getting and proceseing");  

  int tempResist = analogRead(DTM_TEMP);
  float tempVoltage = tempResist * (5.0 / 1023.0);

  int humidityResist = analogRead(DTM_RH);
  float humidityVoltage = humidityResist * (5.0 / 1023.0);

  int gasResist = analogRead(DTM_GAS);
  float gasVoltage = humidityResist * (5.0 / 1023.0);

  float finalHumidity = ((humidityVoltage / 5.0) * 100.0); //%
  float finalF =  ((tempVoltage / 5.0) * 100.0) + 40; //sensor range is 40F - 140F
  float finalGas = ((gasVoltage / 5.0) * 2000); //ppm CO2

  handleDataWithGas(finalHumidity, finalF, finalGas, "22DTM51", "Error reading from 22DTM51 sensor.\n");
}

void handleData(float humidity, float temp, char const * sensorName, char const * message) {
  float readings[2] = {humidity, temp};
  int readingSize = (int)( sizeof(readings) / sizeof(readings[0]) );
  if (validateReadings(readings, readingSize, message)) {
    logReadings(humidity, temp, sensorName);
    sendHumidityAndTempRequest(humidity, temp, sensorName);
  }
}

void handleDataWithGas(float humidity, float temp, float ppm, char const * sensorName, char const * message) {
  float readings[3] = {humidity, temp, ppm};
  int readingSize = (int)( sizeof(readings) / sizeof(readings[0]) );
  if (validateReadings(readings, readingSize, message)) {
    logReadings(humidity, temp, sensorName);
    sendHumidityAndTempAndGasRequest(humidity, temp, ppm, sensorName);
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

ZBTxRequest sendHumidityAndTempRequest(float humidity, float temp, char const * sensorName) {
  if (xbeeEnabled) {
    unsigned long intHumidity = (unsigned long) (humidity * 100.0);
    unsigned long intC = (unsigned long) (temp * 100.0);

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

ZBTxRequest sendHumidityAndTempAndGasRequest(float humidity, float temp, float ppm, char const * sensorName) {
  if (xbeeEnabled) {
    unsigned long intHumidity = (unsigned long) (humidity * 100.0);
    unsigned long intC = (unsigned long) (temp * 100.0);
    unsigned long intPpm = (unsigned long) (ppm * 100.0);

    char charC[numDigits(intC) + 1];
    char charHumidity[numDigits(intHumidity) + 1];
    char charPpm[numDigits(intPpm) + 1];
    ltoa(intC, charC, 10);
    ltoa(intHumidity, charHumidity, 10);
    ltoa(intPpm, charPpm, 10);

    int totalMessageLength = numDigits(intHumidity) + numDigits(intC) + numDigits(intPpm) + strlen(sensorName) + 3; //total length with 2 delimiters and null
    char totalMessage[totalMessageLength] = {0};
    strcat(totalMessage, sensorName);
    strcat(totalMessage, ",");
    strcat(totalMessage, charHumidity);
    strcat(totalMessage, ",");
    strcat(totalMessage, charC);
    strcat(totalMessage, ",");
    strcat(totalMessage, charPpm);
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

boolean validateReadings(float readings[], int readingSize, char const * message) {
  // Check if any reads failed and exit early (to try again).
  for (int i = 0; i < readingSize; i++) {
    if (isnan(readings[i])) {
      digitalWrite(RXLED, LOW);   // set the LED on
      Serial.println(message);    
      return false;
    }
  }  
  digitalWrite(RXLED, HIGH);   // set the LED on
  return true;
}

void logReadings(float humidity, float temp, char const * sensorName) {
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
  return ZBTxRequest(addr, payload, strlen((const char * )payload));
}
