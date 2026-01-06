#include <Arduino.h>
#include <FastLED.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <esp_now.h>

#include "GameEngine.h"
#include "hardware_config.h"

CRGB leds[NUM_LEDS];

GameEngine gameEngine;

// Message types
#define MSG_TYPE_CONNECT 0
#define MSG_TYPE_STATUS 1
#define MSG_TYPE_DISCONNECT 2

typedef struct struct_message {
    uint8_t id;
    uint8_t messageType;  // 0 = CONNECT, 1 = STATUS, 2 = DISCONNECT
    bool isCalibrated;
} struct_message;

struct_message incomingData;

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

// Forward declarations
void updateStatusLEDs(int row, bool isAvailable, bool isCalibrated);
int getLEDIndex(int col, int row);
void handleDeviceOnline(int deviceIndex, uint8_t deviceId, bool calibrated);
void handleDeviceOffline(int deviceIndex, uint8_t deviceId);
void handleCalibrationChange(int deviceIndex, uint8_t deviceId, bool calibrated);
void handleDataReceived(const uint8_t* mac, const uint8_t* incomingDataRaw, int len);

TFT_eSPI tft;

void enableBacklight() {
  digitalWrite(TFT_BL, HIGH);
}

void setup() {
  Serial.begin(115200);

  FastLED.addLeds<WS2812, LED_DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(25);
  FastLED.clear(true);
  FastLED.show();

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
    updateStatusLEDs(i, false, false);
    tft.drawString("Device " + String(KNOWN_DEVICE_IDS[i]) + ": OFFLINE", 10, 30 + i * 20);
  }
  Serial.println("Waiting for devices to connect...");
}

void handleDataReceived(const uint8_t* mac, const uint8_t* incomingDataRaw, int len) {
  memcpy(&incomingData, incomingDataRaw, sizeof(incomingData));

  Serial.print("Data received from MAC: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", mac[i]);
    if (i < 5)
      Serial.print(":");
  }
  Serial.println();

  Serial.print("ESP ID: ");
  Serial.println(incomingData.id);
  Serial.print("Message Type: ");
  Serial.println(incomingData.messageType);
  Serial.print("Calibrated: ");
  Serial.println(incomingData.isCalibrated ? "Yes" : "No");
  Serial.println("---");

  // Update device state
  int deviceIndex = getDeviceIndex(incomingData.id);
  if (deviceIndex < 0) {
    return;  // Unknown device
  }

  bool wasAvailable = deviceStates[deviceIndex].available;
  bool wasCalibrated = deviceStates[deviceIndex].calibrated;

  // Handle different message types
  switch (incomingData.messageType) {
    case MSG_TYPE_CONNECT:
      deviceStates[deviceIndex].available = true;
      deviceStates[deviceIndex].calibrated = incomingData.isCalibrated;
      deviceStates[deviceIndex].lastSeen = millis();
      handleDeviceOnline(deviceIndex, incomingData.id, incomingData.isCalibrated);
      break;

    case MSG_TYPE_STATUS:
      deviceStates[deviceIndex].available = true;
      deviceStates[deviceIndex].calibrated = incomingData.isCalibrated;
      deviceStates[deviceIndex].lastSeen = millis();

      // Only trigger handlers if state actually changed
      if (!wasAvailable) {
        handleDeviceOnline(deviceIndex, incomingData.id, incomingData.isCalibrated);
      } else if (wasCalibrated != incomingData.isCalibrated) {
        handleCalibrationChange(deviceIndex, incomingData.id, incomingData.isCalibrated);
      }
      break;

    case MSG_TYPE_DISCONNECT:
      deviceStates[deviceIndex].available = false;
      deviceStates[deviceIndex].calibrated = false;
      handleDeviceOffline(deviceIndex, incomingData.id);
      break;
  }
}

void loop() {
  // gameEngine.loop();

  unsigned long currentMillis = millis();
}

void handleDeviceOnline(int deviceIndex, uint8_t deviceId, bool calibrated) {
  // Update LEDs
  updateStatusLEDs(deviceIndex, true, calibrated);

  // Print to terminal
  Serial.printf("Device (%d): ONLINE - Calibrated: %s\n", deviceId, calibrated ? "TRUE" : "FALSE");

  // Update TFT
  tft.fillRect(0, 30 + deviceIndex * 20, tft.width(), 20, TFT_BLACK);
  tft.drawString("Device " + String(deviceId) +
                     ": ONLINE - Calibrated: " + String(calibrated ? "TRUE" : "FALSE"),
                 10, 30 + deviceIndex * 20);
}

void handleCalibrationChange(int deviceIndex, uint8_t deviceId, bool calibrated) {
  // Update LEDs
  updateStatusLEDs(deviceIndex, true, calibrated);

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
  updateStatusLEDs(deviceIndex, false, false);

  // Print to terminal
  Serial.printf("Device (%d): OFFLINE\n", deviceId);

  // Update TFT
  tft.fillRect(0, 30 + deviceIndex * 20, tft.width(), 20, TFT_BLACK);
  tft.drawString("Device " + String(deviceId) + ": OFFLINE", 10, 30 + deviceIndex * 20);
}

// Update LED display for a device
// Column 0: Available (Red = offline, Green = online)
// Column 1: Calibrated (Green = TRUE, Orange = FALSE)
void updateStatusLEDs(int row, bool isAvailable, bool isCalibrated) {
  if (!isAvailable) {
    leds[getLEDIndex(0, row)] = CRGB::Red;    // Availability: Red
    leds[getLEDIndex(1, row)] = CRGB::Black;  // Status: Off (no data)
  } else {
    leds[getLEDIndex(0, row)] = CRGB::Green;                                // Availability: Green
    leds[getLEDIndex(1, row)] = isCalibrated ? CRGB::Green : CRGB::Orange;  // Status
  }
  FastLED.show();
}

// Helper function to get LED index from column and row
// Handles serpentine/zigzag wiring pattern
int getLEDIndex(int col, int row) {
  if (row % 2 == 0) {
    // Even rows: left to right
    return row * MATRIX_COLS + col;
  } else {
    // Odd rows: right to left (serpentine)
    return row * MATRIX_COLS + (MATRIX_COLS - 1 - col);
  }
}