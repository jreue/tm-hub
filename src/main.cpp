#include <Arduino.h>
#include <EspNowHelper.h>
#include <MessageStructs.h>

#include "DisplayController.h"
#include "GameEngine.h"
#include "LEDStatusHelper.h"
#include "hardware_config.h"

EspNowHelper espNowHelper;
LEDStatusHelper ledHelper;
DisplayController displayController;

GameEngine gameEngine;

const uint8_t KNOWN_DEVICE_IDS[] = {DEVICE_1_ID, DEVICE_2_ID, DEVICE_3_ID};
const int NUM_DEVICES = sizeof(KNOWN_DEVICE_IDS) / sizeof(KNOWN_DEVICE_IDS[0]);
DeviceState deviceStates[NUM_DEVICES];

// Date state tracking
uint8_t targetMonth = 0, targetDay = 0;
uint16_t targetYear = 0;
uint8_t currentMonth = 1, currentDay = 1;
uint16_t currentYear = 2056;
uint8_t lastMonth = 12, lastDay = 25;
uint16_t lastYear = 2025;

// Intercept window timer (starts at 48 hours in seconds)
int interceptWindowSeconds = 48 * 60 * 60;  // 48 hours = 172800 seconds
unsigned long lastInterceptUpdate = 0;
int interceptHours = 48;
int interceptMinutes = 0;
int interceptSeconds = 0;

// ESP-NOW message flags (volatile because accessed in ISR)
volatile bool dateMessagePending = false;
DateMessage pendingDateMessage;

// Button debouncing
int lastButtonState = HIGH;
int buttonState = HIGH;
unsigned long lastDebounceTime = 0;
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
void updateDeviceStatusDisplay();
void updateDeviceStateOnScanner(DeviceMessage msg);
void checkTravelButton();
void handleTravelButtonPress();

void setup() {
  Serial.begin(115200);
  Serial.println("HUB Starting...");

  Serial.printf("Travel Button PIN: %d\n", TRAVEL_BUTTON_PIN);
  pinMode(TRAVEL_BUTTON_PIN, INPUT_PULLUP);

  ledHelper.begin();
  displayController.begin(NUM_DEVICES, currentMonth, currentDay, currentYear, lastMonth, lastDay,
                          lastYear);

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

  initDevices();
}

void loop() {
  // Process pending ESP-NOW messages first
  if (dateMessagePending) {
    dateMessagePending = false;
    // Store the incoming date as the target date
    targetMonth = pendingDateMessage.month;
    targetDay = pendingDateMessage.day;
    targetYear = pendingDateMessage.year;
    displayController.updateTargetDate(targetMonth, targetDay, targetYear);
  }

  checkTravelButton();

  ledHelper.animate();

  refreshInterceptWindow();

  delay(10);  // Small delay to avoid busy loop
}

void refreshInterceptWindow() {
  unsigned long currentMillis = millis();

  // Update intercept window countdown every second
  if (currentMillis - lastInterceptUpdate >= 1000) {
    lastInterceptUpdate = currentMillis;

    if (interceptWindowSeconds > 0) {
      interceptWindowSeconds--;

      // Convert seconds to hours, minutes, seconds
      int newHours = interceptWindowSeconds / 3600;
      int newMinutes = (interceptWindowSeconds % 3600) / 60;
      int newSeconds = interceptWindowSeconds % 60;

      // Determine what changed
      bool hoursChanged = (newHours != interceptHours);
      bool minutesChanged = (newMinutes != interceptMinutes);

      // Update stored values
      interceptHours = newHours;
      interceptMinutes = newMinutes;
      interceptSeconds = newSeconds;

      // Update display (seconds always change)
      displayController.updateInterceptWindow(interceptHours, interceptMinutes, interceptSeconds,
                                              hoursChanged, minutesChanged);
    }
  }
}

void checkTravelButton() {
  // Simple button debouncing
  int reading = digitalRead(TRAVEL_BUTTON_PIN);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == LOW) {
        handleTravelButtonPress();
      }
    }
  }

  lastButtonState = reading;
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

  bool wasAvailable = deviceStates[deviceIndex].available;
  bool wasCalibrated = deviceStates[deviceIndex].calibrated;

  // Handle different message types
  switch (msg.messageType) {
    case MSG_TYPE_CONNECT:
      deviceStates[deviceIndex].available = true;
      deviceStates[deviceIndex].calibrated = msg.isCalibrated;
      deviceStates[deviceIndex].lastSeen = millis();
      handleDeviceOnline(deviceIndex, msg.deviceId, msg.isCalibrated);
      break;
    case MSG_TYPE_DATA:
      deviceStates[deviceIndex].available = true;
      deviceStates[deviceIndex].calibrated = msg.isCalibrated;
      deviceStates[deviceIndex].lastSeen = millis();

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
  updateDeviceStatusDisplay();
}

void handleDeviceCalibrationChange(int deviceIndex, uint8_t deviceId, bool calibrated) {
  Serial.printf("Device (%d): Changed to %s\n", deviceId, calibrated ? "TRUE" : "FALSE");

  ledHelper.updateStatusLEDs(deviceIndex, true, calibrated);
  updateDeviceStatusDisplay();
}

void handleTravelButtonPress() {
  Serial.println(">>> Travel button pressed! <<<");

  // Roll the dates: last <- current <- target, and reset target
  lastMonth = currentMonth;
  lastDay = currentDay;
  lastYear = currentYear;

  currentMonth = targetMonth;
  currentDay = targetDay;
  currentYear = targetYear;

  targetMonth = 0;
  targetDay = 0;
  targetYear = 0;

  // Update the display with the new dates
  displayController.updateLastDeparture(lastMonth, lastDay, lastYear);
  displayController.updateCurrentDate(currentMonth, currentDay, currentYear);
  displayController.updateTargetDate(targetMonth, targetDay, targetYear);

  Serial.printf(
      "Dates rolled - Last: %02d/%02d/%04d, Current: %02d/%02d/%04d, Target: --/--/----\n",
      lastMonth, lastDay, lastYear, currentMonth, currentDay, currentYear);
}

void initDevices() {
  for (int i = 0; i < NUM_DEVICES; i++) {
    deviceStates[i].available = false;
    deviceStates[i].calibrated = false;
    ledHelper.updateStatusLEDs(i, false, false);
  }
  updateDeviceStatusDisplay();
}

void updateDeviceStatusDisplay() {
  int onlineCount = 0;
  int calibratedCount = 0;

  for (int i = 0; i < NUM_DEVICES; i++) {
    if (deviceStates[i].available) {
      onlineCount++;
      if (deviceStates[i].calibrated) {
        calibratedCount++;
      }
    }
  }

  displayController.updateShieldModules(onlineCount, calibratedCount);

  // updateDeviceStateOnScanner();
}

void updateDeviceStateOnScanner(DeviceMessage msg) {
  Serial.println("Sending device state update to scanner...");

  esp_now_send(scannerMacAddress, (uint8_t*)&msg, sizeof(msg));
}