#include "heltec.h"
#include <WiFi.h>
#include <WiFiMulti.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define WIFISSID ""
#define WIFIPASS ""
#define MQTTSERVER ""
#define MQTTPORT 1883
#define BAND    915E6

WiFiMulti WiFiMulti;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

DynamicJsonDocument packet(1000);

void setup() {
    Serial.begin(115200);
    delay(10);

    Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
    
    initWifi();
    initMqtt();
    initDisplay();
}

void messageLog(String msg, int line=0) {
    if(line == 0) {
      Heltec.display->clear();
    }
    Heltec.display->setColor(WHITE);
    Heltec.display->drawString(0, line*12, msg);
    Heltec.display->display();
    Serial.println(msg);
}

void initWifi() {
    WiFiMulti.addAP(WIFISSID, WIFIPASS);
    messageLog("Connecting to WiFi...");
    messageLog("SSID: "+String(WIFISSID), 1);

    int wifiCounter = 0;
    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(500);
    }

    messageLog("Connected to WiFi");
    messageLog("SSID: "+String(WIFISSID), 1);
    Serial.println(WiFi.localIP());
    delay(1000);
}

void initMqtt() {
  mqttClient.setServer(MQTTSERVER, MQTTPORT);
  mqttReconnect();
  delay(1000);
}

void initDisplay() {
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_10);
}

void initLora() {
  
}

void clearDisplay() {
  Heltec.display->clear();
  Heltec.display->setColor(WHITE);
  Heltec.display->drawString(0, 0, "Listening...");
  Heltec.display->display();
}

void mqttReconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    messageLog("Connecting to MQTT...");
    messageLog(String(MQTTSERVER)+":"+String(MQTTPORT), 1);
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
  messageLog("Connected to MQTT server");
  messageLog(String(MQTTSERVER)+":"+String(MQTTPORT), 1);
}

char message[256]; // max LoRa packet length

unsigned long receivedTimestamp = millis();

void loop() {
    if(!mqttClient.connected()) {
      mqttReconnect();
    }
  
    int packetSize = LoRa.parsePacket();
    if(packetSize) {
        digitalWrite(25, HIGH);

        Serial.print("Received packet '");
        // read packet into string
        int i = 0;
        while (LoRa.available() && i <= 255) {
            message[i++] = (char)LoRa.read();
        }
        message[i] = '\0';

        // send to MQTT
        mqttClient.publish("lora/msg", message);

        Serial.println(String(message)+"' with RSSI "+String(LoRa.packetRssi()));

        // parse the JSON
        DeserializationError err = deserializeJson(packet, message);
        if(err) {
          messageLog("Packet data was not JSON");
        } else {
          const char* username = packet["username"].as<char*>();
          const char* device = packet["device"].as<char*>();
          const char* uniqueid = packet["id"].as<char*>();
          messageLog("username: "+String(username));
          messageLog("device: "+String(device), 1);
          messageLog("id: "+String(uniqueid), 2);
        }

        messageLog("packet length: "+String(packetSize), 3);
        messageLog("RSSI: "+String(LoRa.packetRssi()), 4);
        
        delay(50); // delay before turning LED off
        digitalWrite(25, LOW);
        receivedTimestamp = millis();
    }

    // show the message on the screen for 5 seconds
    if(millis() > receivedTimestamp + 5000) {
      clearDisplay();
    }
}
