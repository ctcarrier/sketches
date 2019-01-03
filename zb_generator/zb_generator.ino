#include <SoftwareSerial.h>
#include <XBee.h>

int RXLED = 8;  // The RX LED has a defined Arduino pin
// The TX LED was not so lucky, we'll need to use pre-defined
// macros (TXLED1, TXLED0) to control that.
// (We could use the same macros for the RX LED too -- RXLED1,
//  and RXLED0.)

#define RXPIN 5
#define TXPIN 6

XBee xbee = XBee();
ZBTxStatusResponse txStatus = ZBTxStatusResponse();
SoftwareSerial mySerial(RXPIN, TXPIN);

void setup()
{
 pinMode(RXLED, OUTPUT);  // Set RX LED as an output
 // TX LED is set as an output behind the scenes

 Serial.begin(9600); //This pipes to the serial monitor 
 mySerial.begin(9600);
 xbee.setSerial(mySerial);
}

void loop()
{
 Serial.println("Hello world");  // Print "Hello World" to the Serial Monitor 

 handleXbeeMessage(getRequest());
 delay(10000);              // wait for a second
}

ZBTxRequest getApiRequest(uint8_t *payload) {
  //uint64_t addr64 = 0x0013A2004147284D;
  XBeeAddress64 addr = XBeeAddress64(0x00000000, 0x00000000);
  uint8_t payload2[] = { 0, 0 };
  return ZBTxRequest(addr, payload, strlen(payload));
}

ZBTxRequest getRequest() {  
  char totalMessage[2] = {1};   
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
