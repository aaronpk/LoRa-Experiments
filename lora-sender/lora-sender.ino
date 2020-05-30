#include "heltec.h"

#define BAND    915E6
#define USERNAME "aaronpk"
#define DEVICE   "bike"

int counter = 0;

void setup() {
  
  //WIFI Kit series V1 not support Vext control
  Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
  
}

void loop() {
  Serial.print("Sending packet: ");
  Serial.println(counter);

  LoRa.beginPacket();

  LoRa.setTxPower(14, RF_PACONFIG_PASELECT_PABOOST);

  LoRa.print("{\"username\":\"");
  LoRa.print(USERNAME);
  LoRa.print("\",\"device\":\"");
  LoRa.print(DEVICE);
  LoRa.print("\",\"id\":");
  LoRa.print(counter);
  LoRa.print("}");
  LoRa.endPacket();
  
  counter++;
  digitalWrite(25, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(25, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
}
