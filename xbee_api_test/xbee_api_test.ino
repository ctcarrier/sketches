#include <SoftwareSerial.h>
#include <XBee.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>

#define rxPin 2
#define txPin 3

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
SoftwareSerial mySerial =  SoftwareSerial(rxPin,txPin);
XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
// create reusable response objects for responses we expect to handle 
Rx16Response rx16 = Rx16Response();
Rx64Response rx64 = Rx64Response();


uint8_t option = 0;
uint8_t data = 0;

void setup()  {
  pinMode(13, OUTPUT);
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  Serial.begin(9600);  
  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);

  xbee.setSerial(mySerial);

  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
}

void wipeData(char *data) {
  for( int i = 0; i < sizeof(data);  ++i )
   data[i] = (char)0;
}

void writeToLcd(char *h, char *t) {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Hum: ");
  lcd.print(h);  
  lcd.setCursor(0,1);        
  lcd.print("Temp: ");
  lcd.print(t);  
}

void loop() {
    
    xbee.readPacket();
    
    if (xbee.getResponse().isAvailable()) {            
      if (xbee.getResponse().getApiId() == RX_16_RESPONSE || xbee.getResponse().getApiId() == RX_64_RESPONSE) {
        // got a rx packet        
        
        if (xbee.getResponse().getApiId() == RX_16_RESPONSE) {   
          xbee.getResponse().getRx16Response(rx16);
          option = rx16.getOption();
          data = rx16.getData(0);
        } else {          
          xbee.getResponse().getRx64Response(rx64);
          option = rx64.getOption();
          char buffer[rx64.getDataLength()];
          char h[rx64.getDataLength()];
          char t[rx64.getDataLength()];
          int bufferCounter = 0;
          Serial.print("DataLength: ");
          Serial.println(rx64.getDataLength());
          for (int i = 0; i < rx64.getDataLength(); i++) {              
            if ((char)rx64.getData(i) == ',') {
              buffer[bufferCounter] = '\0';
              Serial.println(buffer);                  
              strncpy(h, buffer, bufferCounter + 1);
              Serial.println(h);
              Serial.println(sizeof(buffer));
              wipeData(buffer);          
              bufferCounter = 0;
            }            
            else {
              //Serial.print((char)rx64.getData(i));
              buffer[bufferCounter++] = (char)rx64.getData(i);    
            }
          } 
          Serial.println(buffer);    
          buffer[bufferCounter] = '\0';
          strncpy(t, buffer, bufferCounter + 1);
          Serial.println(t);
          Serial.println(sizeof(buffer));
          Serial.print("Humidity: ");
          Serial.println(h);
          Serial.print("Temperature: ");
          Serial.println(t);
          writeToLcd(h, t);
        }                        
      } else {
        Serial.println("Error in api Rx");
      }
    } else if (xbee.getResponse().isError()) {
      Serial.println("Error in api Rx2");
    } 
}
