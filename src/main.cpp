#include <Arduino.h>
#include <ArduinoJson.h>

#include "GameEngine.h"
#include "hardware_config.h"

GameEngine gameEngine;
String serialBuffer = "";

void handleModule1Message(bool status) {
  Serial.println("Handling Module 1 Message");
  Serial.printf("Module 1: %s\n", status ? "true" : "false");
  digitalWrite(MODULE1_LED_PIN, status ? HIGH : LOW);
}

void handleModule2Message(bool status) {
  Serial.println("Handling Module 2 Message");
  Serial.printf("Module 2: %s\n", status ? "true" : "false");
  digitalWrite(MODULE2_LED_PIN, status ? HIGH : LOW);
}

void checkSerialMessages() {
  while (Serial2.available()) {
    char c = Serial2.read();
    Serial.println(serialBuffer);  // Echo received characters for debugging
    if (c == '\n') {
      // Parse complete JSON message
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, serialBuffer);

      if (!error) {
        int id = doc["id"];
        bool status = doc["status"];

        if (id == 1) {
          handleModule1Message(status);
        } else if (id == 2) {
          handleModule2Message(status);
        } else {
          Serial.printf("Unknown module ID: %d\n", id);
        }
      } else {
        Serial.printf("JSON parse error: %s\n", error.c_str());
      }

      serialBuffer = "";
    } else {
      serialBuffer += c;
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(UART_BAUD_RATE, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);

  // Initialize LED pins
  pinMode(MODULE1_LED_PIN, OUTPUT);
  pinMode(MODULE2_LED_PIN, OUTPUT);
  pinMode(MODULE3_LED_PIN, OUTPUT);
  pinMode(MODULE4_LED_PIN, OUTPUT);
  pinMode(MODULE5_LED_PIN, OUTPUT);
  pinMode(MODULE6_LED_PIN, OUTPUT);
  digitalWrite(MODULE1_LED_PIN, LOW);
  digitalWrite(MODULE2_LED_PIN, LOW);
  digitalWrite(MODULE3_LED_PIN, LOW);
  digitalWrite(MODULE4_LED_PIN, LOW);
  digitalWrite(MODULE5_LED_PIN, LOW);
  digitalWrite(MODULE6_LED_PIN, LOW);

  gameEngine.initialize();
  gameEngine.start();  // Will eventualy be triggered by scanner

  Serial.println("tm-hub started");
  Serial.println("Awaiting messages from modules...");

  // Test: Send test messages after 2 seconds

  //  Serial.println("Waiting 5 seconds before sending test messages...");
  // delay(5000);
  // Serial.println("Sending loopback test messages...");

  // Serial2.println("{\"id\":2,\"status\":true}");
}

void loop() {
  gameEngine.loop();
  checkSerialMessages();
}