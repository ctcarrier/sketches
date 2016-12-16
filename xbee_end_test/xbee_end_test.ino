#include <Wire.h>
#include <SoftwareSerial.h>
#include <XBee.h>

XBee xbee = XBee();
ZBTxStatusResponse txStatus = ZBTxStatusResponse();

void setup() { 
  pinMode(13, OUTPUT);
  Serial1.begin(9600);
  Serial.begin(9600);
  //xbee.setSerial(Serial1);
  xbee.setSerial(Serial1);
}

ZBTxRequest getApiRequest() {
  //uint64_t addr64 = 0x0013A2004147284D;
  //XBeeAddress64 addr = XBeeAddress64(addr64); 
  //XBeeAddress64 addr = XBeeAddress64(0x0013A2004104644F);
  //XBeeAddress64 addr = XBeeAddress64(0x00000000, 0x0000ffff);
  XBeeAddress64 addr = XBeeAddress64(0x00000000, 0x00000000);
  uint8_t payload2[5] = { 'h', 'e', 'l', 'l', 'o' };
  Serial.println("Size of ");
  Serial.println(sizeof(payload2));
  return ZBTxRequest(addr, payload2, sizeof(payload2));
}

void loop() {
  ZBTxRequest req = getApiRequest();
  xbee.send(req);
  delay(5000);
}
