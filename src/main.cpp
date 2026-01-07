#include <Arduino.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <esp_now.h>

#include "GameEngine.h"
#include "LEDStatusHelper.h"
#include "hardware_config.h"

LEDStatusHelper ledHelper;

GameEngine gameEngine;

// Message types
#define MSG_TYPE_CONNECT 0
#define MSG_TYPE_STATUS 1
#define MSG_TYPE_DISCONNECT 2
#define MSG_TYPE_DATE_UPDATE 3
#define MSG_TYPE_SCANNER_CONNECTED 4

// Common header for all messages
struct EspNowHeader {
    uint8_t id;
    uint8_t messageType;
};

// Device message (connection/calibration status)
struct DeviceMessage {
    uint8_t id;
    uint8_t messageType;
    bool isCalibrated;
};

// Scanner message
struct ScannerMessage {
    uint8_t id;
    uint8_t messageType;
};

// Date module message
struct DateMessage {
    uint8_t id;
    uint8_t messageType;
    uint8_t month;
    uint8_t day;
    uint16_t year;
};

// Device state tracking
struct DeviceState {
    bool available = false;
    bool calibrated = false;
    unsigned long lastSeen = 0;
};

const uint8_t KNOWN_DEVICE_IDS[] = {DEVICE_1_ID, DEVICE_2_ID, DEVICE_3_ID};
const int NUM_DEVICES = sizeof(KNOWN_DEVICE_IDS) / sizeof(KNOWN_DEVICE_IDS[0]);
DeviceState deviceStates[NUM_DEVICES];

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

// Forward declarations
void handleDataReceived(const uint8_t* mac, const uint8_t* incomingDataRaw, int len);

void handleDeviceMessage(DeviceMessage msg);
void handleDeviceOnline(int deviceIndex, uint8_t deviceId, bool calibrated);
void handleDeviceOffline(int deviceIndex, uint8_t deviceId);
void handleDeviceCalibrationChange(int deviceIndex, uint8_t deviceId, bool calibrated);
void handleDateChanged(uint8_t month, uint8_t day, uint16_t year);

void handleScannerMessage(ScannerMessage msg);
void handleScannerConnected();

TFT_eSPI tft;

void enableBacklight() {
  digitalWrite(TFT_BL, HIGH);
}

void setup() {
  Serial.begin(115200);

  // LED Initialization
  ledHelper.begin();

  // TFT Initialization
  tft.init();
  tft.setRotation(0);  // 1 = landscape, 0 = portrait
  enableBacklight();
  tft.fillScreen(TFT_BLACK);
  tft.drawString("TM Hub", 10, 10);

  WiFi.mode(WIFI_STA);
  Serial.println(WiFi.macAddress());

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(handleDataReceived);

  Serial.println("Waiting for incoming messages...");
  Serial.println("================================\n");

  // Initialize device display
  Serial.println("tm-hub started");
  Serial.printf("Monitoring %d known device(s): ", NUM_DEVICES);
  for (int i = 0; i < NUM_DEVICES; i++) {
    Serial.printf("%d", KNOWN_DEVICE_IDS[i]);
    if (i < NUM_DEVICES - 1)
      Serial.print(", ");
  }
  Serial.println();

  // Initialize all devices as not available
  for (int i = 0; i < NUM_DEVICES; i++) {
    deviceStates[i].available = false;  // Start as disconnected
    deviceStates[i].calibrated = false;
    ledHelper.updateStatusLEDs(i, false, false);
    tft.drawString("Device " + String(KNOWN_DEVICE_IDS[i]) + ": OFFLINE", 10, 30 + i * 20);
  }
  Serial.println("Waiting for devices to connect...");
}

void handleDataReceived(const uint8_t* mac, const uint8_t* incomingDataRaw, int len) {
  // Step 1: Read just the header to determine message type
  EspNowHeader header;
  memcpy(&header, incomingDataRaw, sizeof(EspNowHeader));

  Serial.print("Data received from MAC: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", mac[i]);
    if (i < 5)
      Serial.print(":");
  }
  Serial.println();

  Serial.print("ESP ID: ");
  Serial.println(header.id);
  Serial.print("Message Type: ");
  Serial.println(header.messageType);
  Serial.println("---");

  if (header.messageType == MSG_TYPE_CONNECT || header.messageType == MSG_TYPE_STATUS ||
      header.messageType == MSG_TYPE_DISCONNECT) {
    // Step 2: Deserialize device message
    DeviceMessage deviceMsg;
    memcpy(&deviceMsg, incomingDataRaw, sizeof(DeviceMessage));
    handleDeviceMessage(deviceMsg);
  } else if (header.messageType == MSG_TYPE_DATE_UPDATE) {
    // Step 2: Deserialize date message
    DateMessage dateMsg;
    memcpy(&dateMsg, incomingDataRaw, sizeof(DateMessage));
    handleDateChanged(dateMsg.month, dateMsg.day, dateMsg.year);
  } else if (header.messageType == MSG_TYPE_SCANNER_CONNECTED) {
    // Step 2: Deserialize scanner message
    ScannerMessage scannerMsg;
    memcpy(&scannerMsg, incomingDataRaw, sizeof(ScannerMessage));
    handleScannerMessage(scannerMsg);
  } else {
    Serial.println("Unknown message type received.");
  }
}

void handleScannerMessage(ScannerMessage msg) {
  Serial.println("Scanner connected message received.");
  switch (msg.messageType) {
    case MSG_TYPE_SCANNER_CONNECTED:
      handleScannerConnected();
      break;
    default:
      Serial.println("Unknown scanner message type.");
      return;
  }
}

void handleScannerConnected() {
  Serial.println("Scanner has connected!");
  tft.fillRect(0, 200, tft.width(), 20, TFT_BLACK);
  tft.drawString("Scanner: CONNECTED", 10, 200);
}

void handleDeviceMessage(DeviceMessage msg) {
  // Update device state
  int deviceIndex = getDeviceIndex(msg.id);
  if (deviceIndex < 0) {
    return;  // Unknown device
  }

  bool wasAvailable = deviceStates[deviceIndex].available;
  bool wasCalibrated = deviceStates[deviceIndex].calibrated;

  // Handle different message types
  switch (msg.messageType) {
    case MSG_TYPE_CONNECT:
      deviceStates[deviceIndex].available = true;
      deviceStates[deviceIndex].calibrated = msg.isCalibrated;
      deviceStates[deviceIndex].lastSeen = millis();
      handleDeviceOnline(deviceIndex, msg.id, msg.isCalibrated);
      break;

    case MSG_TYPE_STATUS:
      deviceStates[deviceIndex].available = true;
      deviceStates[deviceIndex].calibrated = msg.isCalibrated;
      deviceStates[deviceIndex].lastSeen = millis();

      // Only trigger handlers if state actually changed
      if (!wasAvailable) {
        handleDeviceOnline(deviceIndex, msg.id, msg.isCalibrated);
      } else if (wasCalibrated != msg.isCalibrated) {
        handleDeviceCalibrationChange(deviceIndex, msg.id, msg.isCalibrated);
      }
      break;

    case MSG_TYPE_DISCONNECT:
      deviceStates[deviceIndex].available = false;
      deviceStates[deviceIndex].calibrated = false;
      handleDeviceOffline(deviceIndex, msg.id);
      break;
  }
}

void loop() {
  // gameEngine.loop();

  // Animate calibrated devices
  ledHelper.animate();

  unsigned long currentMillis = millis();
}

void handleDateChanged(uint8_t month, uint8_t day, uint16_t year) {
  // Print to terminal
  Serial.printf("Date Update Received: %02d/%02d/%04d\n", month, day, year);

  // Update TFT (at bottom of 320x480 portrait display)
  tft.setTextSize(2);
  tft.fillRect(0, 440, tft.width(), 40, TFT_BLACK);
  tft.drawString("Date: " + String(month) + "/" + String(day) + "/" + String(year), 10, 450);
  tft.setTextSize(1);  // Reset to default size
}

void handleDeviceOnline(int deviceIndex, uint8_t deviceId, bool calibrated) {
  // Update LEDs
  ledHelper.updateStatusLEDs(deviceIndex, true, calibrated);

  // Print to terminal
  Serial.printf("Device (%d): ONLINE - Calibrated: %s\n", deviceId, calibrated ? "TRUE" : "FALSE");

  // Update TFT
  tft.fillRect(0, 30 + deviceIndex * 20, tft.width(), 20, TFT_BLACK);
  tft.drawString("Device " + String(deviceId) +
                     ": ONLINE - Calibrated: " + String(calibrated ? "TRUE" : "FALSE"),
                 10, 30 + deviceIndex * 20);
}

void handleDeviceCalibrationChange(int deviceIndex, uint8_t deviceId, bool calibrated) {
  // Update LEDs
  ledHelper.updateStatusLEDs(deviceIndex, true, calibrated);

  // Print to terminal
  Serial.printf("Device (%d): Calibration changed to %s\n", deviceId,
                calibrated ? "TRUE" : "FALSE");

  // Update TFT
  tft.fillRect(0, 30 + deviceIndex * 20, tft.width(), 20, TFT_BLACK);
  tft.drawString("Device " + String(deviceId) +
                     ": ONLINE - Calibrated: " + String(calibrated ? "TRUE" : "FALSE"),
                 10, 30 + deviceIndex * 20);
}

void handleDeviceOffline(int deviceIndex, uint8_t deviceId) {
  // Update LEDs
  ledHelper.updateStatusLEDs(deviceIndex, false, false);

  // Print to terminal
  Serial.printf("Device (%d): OFFLINE\n", deviceId);

  // Update TFT
  tft.fillRect(0, 30 + deviceIndex * 20, tft.width(), 20, TFT_BLACK);
  tft.drawString("Device " + String(deviceId) + ": OFFLINE", 10, 30 + deviceIndex * 20);
}