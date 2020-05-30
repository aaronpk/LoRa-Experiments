#include "heltec.h"
#include <WiFi.h>
#include <WiFiMulti.h>
#include <PubSubClient.h>

#define WIFISSID ""
#define WIFIPASS ""
#define MQTTSERVER ""
#define MQTTPORT 1883
#define BAND    915E6

WiFiMulti WiFiMulti;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void setup() {
    Serial.begin(115200);
    delay(10);

    Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
    
    initWifi();
    initMqtt();
}

void messageLog(const char *msg) {
//    display.clear();
//    display.setTextAlignment(TEXT_ALIGN_LEFT);
//    display.setFont(ArialMT_Plain_10);
//    display.drawString(0, 0, msg);
//    display.display();
    Serial.println(msg);
}

void initWifi() {
    WiFiMulti.addAP(WIFISSID, WIFIPASS);
    messageLog("Connecting to WiFi... ");

    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(500);
        messageLog("Connecting to WiFi... ");
    }

    messageLog("Connected to Wifi");
    Serial.println(WiFi.localIP());
    delay(100);
}

void initMqtt() {
  mqttClient.setServer(MQTTSERVER, MQTTPORT);
  mqttReconnect();
  messageLog("Connected to MQTT... ");
  delay(1000);  
}

void initLora() {
  
}

void mqttReconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect("lora")) {
      Serial.println("connected");
      mqttClient.publish("lora/init", "connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

char message[4000];

void loop() {
    if(!mqttClient.connected()) {
      mqttReconnect();
    }
  
    // try to parse packet
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        // received a packet
        Serial.print("Received packet '");
        // read packet
        int i = 0;
        while (LoRa.available()) {
            message[i++] = (char)LoRa.read();
        }
        message[i] = '\0';
        mqttClient.publish("lora/msg", message);
        Serial.print(message);
        Serial.print("' with RSSI ");
        Serial.println(LoRa.packetRssi());
      }
}
