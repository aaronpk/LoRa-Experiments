#include "heltec.h"
#include <WiFi.h>
#include <WiFiMulti.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

/* wifi credentials */
#define WIFISSID ""
#define WIFIPASS ""
/* point this at your MQTT server */
#define MQTTSERVER "10.10.33.33"
#define MQTTPORT 1883
/* 915E6 is 915Mhz band for the US */
#define BAND    915E6
/* identifier for this receiver */
#define STATIONID ""

/* 
 * MQTT messages will be formatted as:  
 * topic: lora/msg/[station]/[rssi]
 * payload: passthru from receiver, unparsed
 */

WiFiMulti WiFiMulti;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

DynamicJsonDocument packet(1000);

void setup() {
    Serial.begin(115200);
    delay(10);

    initLora();
    initWifi();
    initMqtt();
    initDisplay();
    showWaitingMessage();
}

void drawMessage(String msg, int line=0) {
    if(line == 0) {
      Heltec.display->clear();
      Heltec.display->setColor(WHITE);
    }
    Heltec.display->drawString(0, line*12, msg);
}

void displayMessage() {
    Heltec.display->display();
}

void initLora() {
    Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
    LoRa.setSpreadingFactor(7);
    LoRa.setSignalBandwidth(125E3);
}

void initWifi() {
    WiFiMulti.addAP(WIFISSID, WIFIPASS);
    drawMessage("Connecting to WiFi...");
    drawMessage("SSID: "+String(WIFISSID), 1);
    displayMessage();

    int wifiCounter = 0;
    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(500);
    }

    drawMessage("Connected to WiFi");
    drawMessage("SSID: "+String(WIFISSID), 1);
    drawMessage("IP: "+WiFi.localIP().toString(), 2);
    displayMessage();
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

void showWaitingMessage() {
  drawMessage("Listening for data...");
  drawMessage("WiFi: "+String(WIFISSID), 1);
  drawMessage("MQTT: "+String(MQTTSERVER)+":"+String(MQTTPORT), 2);
  drawMessage("IP: "+WiFi.localIP().toString(), 3);
  drawMessage("Station: "+String(STATIONID), 4);
  displayMessage();
}

void mqttReconnect() {
  Serial.println("Reconnecting to MQTT");
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    drawMessage("Connecting to MQTT...");
    drawMessage(String(MQTTSERVER)+":"+String(MQTTPORT), 1);
    displayMessage();
    // Attempt to connect
    if (mqttClient.connect("lora")) {
      Serial.println("connected to MQTT");
      char mqttkey[100];
      sprintf(mqttkey, "lora/init/%s", STATIONID);
      Serial.println("Publishing "+String(mqttkey));      
      mqttClient.publish(mqttkey, "connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  drawMessage("Connected to MQTT server");
  drawMessage(String(MQTTSERVER)+":"+String(MQTTPORT), 1);
  displayMessage();
}

char message[256]; // max LoRa packet length

unsigned long receivedTimestamp = 0;

void loop() {
    if(!mqttClient.connected()) {
      mqttReconnect();
      showWaitingMessage();
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
        char mqttkey[100];
        sprintf(mqttkey, "lora/msg/%s/%d", STATIONID, LoRa.packetRssi());
        Serial.println("Publishing "+String(mqttkey));
        mqttClient.publish(mqttkey, message);

        Serial.println(String(message)+"' with RSSI "+String(LoRa.packetRssi()));

        // parse the JSON
        DeserializationError err = deserializeJson(packet, message);
        if(err) {
          drawMessage("Packet data was not JSON");
          displayMessage();
        } else {
          const char* username = packet["username"].as<char*>();
          const char* device = packet["device"].as<char*>();
          const char* uniqueid = packet["id"].as<char*>();
          drawMessage("username: "+String(username));
          drawMessage("device: "+String(device), 1);
          drawMessage("id: "+String(uniqueid), 2);
        }

        drawMessage("packet length: "+String(packetSize), 3);
        drawMessage("RSSI: "+String(LoRa.packetRssi()), 4);
        displayMessage();
        
        delay(50); // delay before turning LED off
        digitalWrite(25, LOW);
        receivedTimestamp = millis();
    }

    // show the message on the screen for 5 seconds
    if(receivedTimestamp != 0 && millis() > receivedTimestamp + 5000) {
      showWaitingMessage();
      receivedTimestamp = 0;
      Serial.println("Resetting display");
    }
}
