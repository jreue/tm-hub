#include <Arduino.h>

#include "DateController.h"
#include "GameEngine.h"

DateController dateController;
GameEngine gameEngine(dateController);

void setup() {
  Serial.begin(115200);
  Serial2.begin(UART_BAUD_RATE, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);

  gameEngine.initialize();
  gameEngine.startGame();  // Will eventualy be triggered by scanner
}

void loop() {
  gameEngine.loop();
}