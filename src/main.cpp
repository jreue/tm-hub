#include <Arduino.h>

#include "DateController.h"
#include "DateDialsController.h"
#include "GameEngine.h"

DateController dateController;
DateDialsController dateDialsController;
GameEngine gameEngine(dateController);

void setup() {
  Serial.begin(115200);
  Serial2.begin(UART_BAUD_RATE, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);

  gameEngine.initialize();
  gameEngine.start();  // Will eventualy be triggered by scanner

  dateController.showDate(12, 25, 1975);  // Example date
}

void loop() {
  gameEngine.loop();

  DateFromDials date = dateDialsController.readDate();

    if (date.changed) {
    Serial.println(String(date.month) + "/" + String(date.day) + "/" + String(date.year));
    dateController.showDate(date.month, date.day, date.year);
  }
}