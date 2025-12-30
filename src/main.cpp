#include <Arduino.h>
#include <FastLED.h>
#include <TFT_eSPI.h>
#include <Wire.h>

#include "GameEngine.h"
#include "I2CBusManager.h"
#include "hardware_config.h"

CRGB leds[NUM_LEDS];

GameEngine gameEngine;
I2CBusManager i2cBusManager;
unsigned long lastDevicePoll = 0;
const unsigned long DEVICE_POLL_INTERVAL = 100;  // Poll every 100ms

// Forward declarations
void handleScanMissing(int index, uint8_t address);
void handleScanFound(int index, uint8_t address);
void handleDeviceChecked(const DeviceStateChange& change);
void handleDeviceOffline(const DeviceStateChange& change);
void handleDeviceOnline(const DeviceStateChange& change);
void updateStatusLEDs(int row, bool isAvailable, bool isCalibrated);
int getLEDIndex(int col, int row);

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

  esp_log_level_set("i2c", ESP_LOG_NONE);  // Suppress I2C logs
  Wire.setTimeOut(50);                     // 50ms timeout for I2C operations
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  Serial.println("tm-hub started (I2C mode)");
  Serial.println("Waiting 2 seconds for devices to initialize...");
  delay(2000);  // Give devices time to boot and initialize I2C

  Serial.printf("tm-hub will now scan for the %d known device(s): ", I2CBusManager::NUM_DEVICES);
  for (int i = 0; i < I2CBusManager::NUM_DEVICES; i++) {
    Serial.printf("0x%02X", I2CBusManager::DEVICE_ADDRESSES[i]);
    if (i < I2CBusManager::NUM_DEVICES - 1)
      Serial.print(", ");
  }
  Serial.println();

  Serial.println("Scanning the I2C bus...");
  auto foundDevices = i2cBusManager.scanBus();
  Serial.printf("Scan complete. Found %d device(s)\n", foundDevices.size());

  // Check each expected device against scan results
  for (int i = 0; i < I2CBusManager::NUM_DEVICES; i++) {
    uint8_t address = I2CBusManager::DEVICE_ADDRESSES[i];
    bool found = false;
    for (auto addr : foundDevices) {
      if (addr == address) {
        found = true;
        break;
      }
    }

    if (found) {
      handleScanFound(i, address);
    } else {
      handleScanMissing(i, address);
    }
  }

  Serial.println("Polling devices...");
}

void loop() {
  // gameEngine.loop();

  unsigned long currentMillis = millis();

  // Check all modules at the defined interval
  if (currentMillis - lastDevicePoll >= DEVICE_POLL_INTERVAL) {
    lastDevicePoll = currentMillis;

    auto stateChanges = i2cBusManager.checkAllDevices();
    for (const auto& change : stateChanges) {
      handleDeviceChecked(change);
    }
  }
}

void handleScanMissing(int index, uint8_t address) {
  // Update LED to red
  updateStatusLEDs(index, false, false);

  // Print to terminal
  Serial.printf("Address 0x%02X not found\n", address);

  // Print to TFT
  tft.drawString("Address 0x" + String(address, HEX) + " not found", 10, 30 + index * 20);
}

void handleScanFound(int index, uint8_t address) {
  // Update LED to green
  updateStatusLEDs(index, true, false);

  // Print to terminal
  Serial.printf("Address 0x%02X found\n", address);

  // Print to TFT
  tft.drawString("Address 0x" + String(address, HEX) + " found", 10, 30 + index * 20);
}

void handleDeviceChecked(const DeviceStateChange& change) {
  updateStatusLEDs(change.index, change.available, change.calibrated);

  if (!change.available) {
    handleDeviceOffline(change);
  } else {
    handleDeviceOnline(change);
  }
}

void handleDeviceOffline(const DeviceStateChange& change) {
  // Print to terminal
  Serial.printf("Device (0x%02X): OFFLINE\n", change.address);

  // Print to TFT
  tft.drawString("Device 0x" + String(change.address, HEX) + ": OFFLINE", 10,
                 30 + change.index * 20);
}

void handleDeviceOnline(const DeviceStateChange& change) {
  // Print to terminal
  Serial.printf("Device (0x%02X): ONLINE - Calibrated: %s\n", change.address,
                change.calibrated ? "TRUE" : "FALSE");

  // Print to TFT
  tft.drawString("Device 0x" + String(change.address, HEX) +
                     ": ONLINE - Calibrated: " + String(change.calibrated ? "TRUE" : "FALSE"),
                 10, 30 + change.index * 20);
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