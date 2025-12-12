#include <Arduino.h>

#include "GameEngine.h"
#include "hardware_config.h"

GameEngine gameEngine;

void setup() {
  Serial.begin(115200);
  Serial2.begin(UART_BAUD_RATE, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);

  gameEngine.initialize();
  gameEngine.start();  // Will eventualy be triggered by scanner

  Serial.println("tm-hub started");
}

void loop() {
  gameEngine.loop();
}