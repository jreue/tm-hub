#include <Arduino.h>
#include <EspNowHelper.h>
#include <MessageStructs.h>

#include "DisplayController.h"
#include "GameEngine.h"
#include "GameState.h"
#include "LEDStatusHelper.h"
#include "hardware_config.h"

EspNowHelper espNowHelper;
LEDStatusHelper ledHelper;
DisplayController displayController;
GameState gameState;
GameEngine gameEngine;

const uint8_t KNOWN_DEVICE_IDS[] = {DEVICE_1_ID, DEVICE_2_ID, DEVICE_3_ID};
const int NUM_DEVICES = sizeof(KNOWN_DEVICE_IDS) / sizeof(KNOWN_DEVICE_IDS[0]);

// Timers
unsigned long lastInterceptUpdate = 0;

// ESP-NOW message flags (volatile because accessed in ISR)
volatile bool dateMessagePending = false;
DateMessage pendingDateMessage;

// Travel button debouncing
int lastTravelButtonState = HIGH;
int travelButtonState = HIGH;
unsigned long lastTravelDebounceTime = 0;

// Reset button debouncing
int lastResetButtonState = HIGH;
int resetButtonState = HIGH;
unsigned long lastResetDebounceTime = 0;

// Shared button debouncing delay
const unsigned long debounceDelay = 50;

// Helper to get device index from ID
int getDeviceIndex(uint8_t deviceId) {
  for (int i = 0; i < NUM_DEVICES; i++) {
    if (KNOWN_DEVICE_IDS[i] == deviceId) {
      return i;
    }
  }
  return -1;  // Not found
}

uint8_t scannerMacAddress[] = SCANNER_MAC_ADDRESS;

// Date Message Handlers
void processPendingDateMessage();
void handleDateChanged(const DateMessage& msg);

// Scanner Message Handlers
void handleScannerMessage(const ScannerMessage& msg);
void handleScannerConnected();

// Device Message Handlers
void handleDeviceMessage(const DeviceMessage& msg);
void handleDeviceOnline(int deviceIndex, uint8_t deviceId, bool calibrated);
void handleDeviceCalibrationChange(int deviceIndex, uint8_t deviceId, bool calibrated);

void refreshInterceptWindow();
void initDevices();
void updateDeviceStateOnHubDisplay();
void updateDeviceStateOnHubLeds();
void updateDeviceStateOnScanner(DeviceMessage msg);
void checkTravelButton();
void handleTravelButtonPress();
void checkResetButton();
void handleResetButtonPress();

void setup() {
  Serial.begin(115200);
  Serial.println("HUB Starting...");

  // Load saved game state from NVS
  gameState.begin();
  gameState.load();

  Serial.printf("Travel Button PIN: %d\n", TRAVEL_BUTTON_PIN);
  pinMode(TRAVEL_BUTTON_PIN, INPUT_PULLUP);

  Serial.printf("Reset Button PIN: %d\n", RESET_BUTTON_PIN);
  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);

  ledHelper.begin();

  uint8_t targetMonth, targetDay, currentMonth, currentDay, lastMonth, lastDay;
  uint16_t targetYear, currentYear, lastYear;
  gameState.getTargetDate(targetMonth, targetDay, targetYear);
  gameState.getCurrentDate(currentMonth, currentDay, currentYear);
  gameState.getLastDate(lastMonth, lastDay, lastYear);
  displayController.begin(NUM_DEVICES, targetMonth, targetDay, targetYear, currentMonth, currentDay,
                          currentYear, lastMonth, lastDay, lastYear);

  Serial.print("Known Device IDs: [");
  for (int i = 0; i < NUM_DEVICES; i++) {
    Serial.printf("%d", KNOWN_DEVICE_IDS[i]);
    if (i < NUM_DEVICES - 1)
      Serial.print(", ");
  }
  Serial.println("]");

  espNowHelper.begin(scannerMacAddress, DEVICE_ID);
  espNowHelper.registerDateMessageHandler(handleDateChanged);
  espNowHelper.registerScannerMessageHandler(handleScannerMessage);
  espNowHelper.registerModuleMessageHandler(handleDeviceMessage);

  // Restore device display states from loaded data
  updateDeviceStateOnHubDisplay();
  updateDeviceStateOnHubLeds();
}

void loop() {
  processPendingDateMessage();

  checkTravelButton();
  checkResetButton();

  ledHelper.animate();

  refreshInterceptWindow();

  delay(10);  // Small delay to avoid busy loop
}

void refreshInterceptWindow() {
  unsigned long currentMillis = millis();

  // Update intercept window countdown every second
  if (currentMillis - lastInterceptUpdate >= 1000) {
    lastInterceptUpdate = currentMillis;

    int interceptWindowSeconds = gameState.getInterceptWindowSeconds();
    if (interceptWindowSeconds > 0) {
      interceptWindowSeconds--;
      gameState.setInterceptWindowSeconds(interceptWindowSeconds);

      // Get time components for display
      int hours, minutes, seconds;
      gameState.getInterceptWindowTime(hours, minutes, seconds);

      // Track changes for display optimization (using static variables)
      static int lastHours = -1;
      static int lastMinutes = -1;

      bool hoursChanged = (hours != lastHours);
      bool minutesChanged = (minutes != lastMinutes);

      lastHours = hours;
      lastMinutes = minutes;

      // Update display
      displayController.updateInterceptWindow(hours, minutes, seconds, hoursChanged,
                                              minutesChanged);

      // Save to NVS when minutes change to reduce flash wear
      if (minutesChanged) {
        gameState.save();
      }
    }
  }
}

void processPendingDateMessage() {
  // Process pending ESP-NOW messages first
  if (dateMessagePending) {
    dateMessagePending = false;
    // Store the incoming date as the target date
    gameState.setTargetDate(pendingDateMessage.month, pendingDateMessage.day,
                            pendingDateMessage.year);
    displayController.updateTargetDate(pendingDateMessage.month, pendingDateMessage.day,
                                       pendingDateMessage.year);
  }
}

void checkTravelButton() {
  // Simple button debouncing
  int reading = digitalRead(TRAVEL_BUTTON_PIN);

  if (reading != lastTravelButtonState) {
    lastTravelDebounceTime = millis();
  }

  if ((millis() - lastTravelDebounceTime) > debounceDelay) {
    if (reading != travelButtonState) {
      travelButtonState = reading;

      if (travelButtonState == LOW) {
        handleTravelButtonPress();
      }
    }
  }

  lastTravelButtonState = reading;
}

void checkResetButton() {
  // Simple button debouncing
  int reading = digitalRead(RESET_BUTTON_PIN);

  if (reading != lastResetButtonState) {
    lastResetDebounceTime = millis();
  }

  if ((millis() - lastResetDebounceTime) > debounceDelay) {
    if (reading != resetButtonState) {
      resetButtonState = reading;

      if (resetButtonState == LOW) {
        handleResetButtonPress();
      }
    }
  }

  lastResetButtonState = reading;
}

void handleDateChanged(const DateMessage& msg) {
  Serial.printf("Date Update Received: %02d/%02d/%04d\n", msg.month, msg.day, msg.year);
  // Don't update display from ISR context - set flag instead
  pendingDateMessage = msg;
  dateMessagePending = true;
}

void handleScannerMessage(const ScannerMessage& msg) {
  Serial.printf("Handling scanner message type: %d\n", msg.messageType);

  switch (msg.messageType) {
    case MSG_TYPE_CONNECT:
      handleScannerConnected();
      break;
    case MSG_TYPE_DATA:
      Serial.println("Received data message from scanner. Nothing to do yet.");
      break;
    default:
      Serial.println("Unknown scanner message type.");
      return;
  }
}

void handleScannerConnected() {
  Serial.println("Scanner has connected!");
  displayController.updateServiceLink(true);
}

void handleDeviceMessage(const DeviceMessage& msg) {
  Serial.printf("Handling device message from ID: %d\n", msg.deviceId);

  int deviceIndex = getDeviceIndex(msg.deviceId);
  if (deviceIndex < 0) {
    Serial.printf("Unknown device ID: %d\n", msg.deviceId);
    return;
  }

  bool wasAvailable, wasCalibrated;
  gameState.getDeviceState(deviceIndex, wasAvailable, wasCalibrated);

  // Handle different message types
  switch (msg.messageType) {
    case MSG_TYPE_CONNECT:
      gameState.setDeviceState(deviceIndex, true, msg.isCalibrated);
      handleDeviceOnline(deviceIndex, msg.deviceId, msg.isCalibrated);
      break;
    case MSG_TYPE_DATA:
      gameState.setDeviceState(deviceIndex, true, msg.isCalibrated);

      // Only trigger handlers if state actually changed
      if (!wasAvailable) {
        handleDeviceOnline(deviceIndex, msg.deviceId, msg.isCalibrated);
      } else if (wasCalibrated != msg.isCalibrated) {
        handleDeviceCalibrationChange(deviceIndex, msg.deviceId, msg.isCalibrated);
      }
      break;
    default:
      Serial.println("Unknown device message type.");
  }

  updateDeviceStateOnScanner(msg);
}

void handleDeviceOnline(int deviceIndex, uint8_t deviceId, bool calibrated) {
  Serial.printf("Device (%d): ONLINE - Calibrated: %s\n", deviceId, calibrated ? "TRUE" : "FALSE");

  ledHelper.updateStatusLEDs(deviceIndex, true, calibrated);
  updateDeviceStateOnHubDisplay();
  gameState.save();
}

void handleDeviceCalibrationChange(int deviceIndex, uint8_t deviceId, bool calibrated) {
  Serial.printf("Device (%d): Changed to %s\n", deviceId, calibrated ? "TRUE" : "FALSE");

  ledHelper.updateStatusLEDs(deviceIndex, true, calibrated);
  updateDeviceStateOnHubDisplay();
  gameState.save();
}

void handleTravelButtonPress() {
  Serial.println(">>> Travel button pressed! <<<");

  // Roll the dates
  gameState.rollDates();

  // Get dates for display update
  uint8_t targetMonth, targetDay, currentMonth, currentDay, lastMonth, lastDay;
  uint16_t targetYear, currentYear, lastYear;
  gameState.getTargetDate(targetMonth, targetDay, targetYear);
  gameState.getCurrentDate(currentMonth, currentDay, currentYear);
  gameState.getLastDate(lastMonth, lastDay, lastYear);

  // Update the display with the new dates
  displayController.updateLastDeparture(lastMonth, lastDay, lastYear);
  displayController.updateCurrentDate(currentMonth, currentDay, currentYear);
  displayController.updateTargetDate(targetMonth, targetDay, targetYear);

  gameState.save();
}

void handleResetButtonPress() {
  Serial.println(">>> Reset button pressed! Clearing NVS... <<<");
  gameState.reset();
  Serial.println("NVS cleared. Restarting...");
  ESP.restart();
}

void initDevices() {
  for (int i = 0; i < NUM_DEVICES; i++) {
    gameState.setDeviceState(i, false, false);
    ledHelper.updateStatusLEDs(i, false, false);
  }
  updateDeviceStateOnHubDisplay();
}

void updateDeviceStateOnHubDisplay() {
  int onlineCount = 0;
  int calibratedCount = 0;

  DeviceState* deviceStates = gameState.getDeviceStates();
  for (int i = 0; i < NUM_DEVICES; i++) {
    if (deviceStates[i].available) {
      onlineCount++;
      if (deviceStates[i].calibrated) {
        calibratedCount++;
      }
    }
  }

  displayController.updateShieldModules(onlineCount, calibratedCount);
}

void updateDeviceStateOnHubLeds() {
  DeviceState* deviceStates = gameState.getDeviceStates();
  for (int i = 0; i < NUM_DEVICES; i++) {
    ledHelper.updateStatusLEDs(i, deviceStates[i].available, deviceStates[i].calibrated);
  }
}

void updateDeviceStateOnScanner(DeviceMessage msg) {
  Serial.println("Sending device state update to scanner...");

  esp_now_send(scannerMacAddress, (uint8_t*)&msg, sizeof(msg));
}