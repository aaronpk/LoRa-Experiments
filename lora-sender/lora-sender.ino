#include "heltec.h"

#define BAND    915E6
#define USERNAME "aaronpk"
#define DEVICE   "bike"

int counter = 0;

void setup() {
  Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_16);
  //LoRa.setSpreadingFactor(6);
  LoRa.enableCrc();
  //LoRa.setSignalBandwidth(125E3);
}

void loop() {
  Serial.print("Sending packet: ");
  Serial.println(counter);

  digitalWrite(25, HIGH);

  // Show the count on the screen
  Heltec.display->clear();
  Heltec.display->setColor(WHITE);
  Heltec.display->drawString(0, 0, String(USERNAME));
  Heltec.display->drawString(0, 16, "device: "+String(DEVICE));
  Heltec.display->drawString(0, 32, "id: "+String(counter));
  Heltec.display->display();

  // Send the LoRa packet
  LoRa.beginPacket();
  LoRa.setTxPower(14, RF_PACONFIG_PASELECT_PABOOST);
  LoRa.print("{\"username\":\""+String(USERNAME)+"\",\"device\":\""+String(DEVICE)+"\",\"id\":\""+String(counter)+"\"}");
  LoRa.endPacket();

  counter++;

//  Heltec.display->setColor(BLACK);
//  Heltec.display->fillRect(112, 16, 8, 8);
//  Heltec.display->display();
  digitalWrite(25, LOW);
  
  delay(2000);
}
