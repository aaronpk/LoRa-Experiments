# LoRa Experiments

## LoRa to MQTT

This project receives packets via LoRa and forwards them to MQTT. You'll need to configure your WiFi credentials and MQTT server.

## LoRa Sender

This project broadcasts LoRa packets at a regular interval. The payload is a JSON string such as:

```json
{
	"username": "aaronpk",
	"device": "bike",
	"id": 20
}
```

The ID increments on every message and resets on boot.

