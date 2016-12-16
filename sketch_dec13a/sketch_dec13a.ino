#include <SoftwareSerial.h>

#define RXPIN 10
#define TXPIN 11

SoftwareSerial mySerial(RXPIN, TXPIN); // RX, TX

void setup() {
  // Open serial communications and wait for port to open:  
  pinMode(RXPIN, INPUT);
  pinMode(TXPIN, OUTPUT);
  Serial.begin(9600);
  
  mySerial.begin(9600);  
}

void loop() { // run over and over
  int got = 0;
  while (mySerial.available()) {
    got = 1;
    delay(10);
    Serial.print(mySerial.read(),HEX);
  }
  if (got == 1) {
    Serial.println("");
    got=0;
  }
}
