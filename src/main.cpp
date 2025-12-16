#include <Arduino.h>
#include <FastLED.h>
#include <Wire.h>

#include "GameEngine.h"
#include "hardware_config.h"

CRGB leds[NUM_LEDS];

GameEngine gameEngine;
unsigned long lastDevicePoll = 0;
const unsigned long DEVICE_POLL_INTERVAL = 100;  // Poll every 100ms

// Device configuration
const uint8_t DEVICE_ADDRESSES[] = {DEVICE_1_ADDR, DEVICE_2_ADDR};
const int NUM_DEVICES = sizeof(DEVICE_ADDRESSES) / sizeof(DEVICE_ADDRESSES[0]);

// Device state tracking
struct DeviceState {
    bool available = false;
    bool calibrated = false;
};
DeviceState deviceStates[NUM_DEVICES];

// Forward declarations
void scanI2CBus();
void checkAllDevices();
void checkDevice(uint8_t address, int displayRow);
bool isDeviceAvailable(uint8_t addr);
void updateStatusLEDs(int row, bool isAvailable, bool isCalibrated);
int getLEDIndex(int col, int row);

void setup() {
  Serial.begin(115200);

  FastLED.addLeds<WS2812, LED_DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(25);
  FastLED.clear(true);
  FastLED.show();

  esp_log_level_set("i2c", ESP_LOG_NONE);  // Suppress I2C logs
  Wire.setTimeOut(50);                     // 50ms timeout for I2C operations
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  Serial.println("tm-hub started (I2C mode)");
  Serial.println("Waiting for devices to initialize...");
  delay(2000);  // Give devices time to boot and initialize I2C
  scanI2CBus();
  Serial.println("Polling devices...");
}

void loop() {
  // gameEngine.loop();

  unsigned long currentMillis = millis();

  // Check all modules at the defined interval
  if (currentMillis - lastDevicePoll >= DEVICE_POLL_INTERVAL) {
    lastDevicePoll = currentMillis;
    checkAllDevices();
  }
}

void scanI2CBus() {
  Serial.println("Scanning I2C bus...");

  int devicesFound = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    uint8_t error = Wire.endTransmission();
    if (error == 0) {
      Serial.printf("Found device at 0x%02X\n", addr);
      devicesFound++;
    }
  }
  Serial.printf("Scan complete. Found %d device(s)\n", devicesFound);
}

void checkAllDevices() {
  for (int i = 0; i < NUM_DEVICES; i++) {
    checkDevice(DEVICE_ADDRESSES[i], i);
  }
}

void checkDevice(uint8_t address, int displayRow) {
  bool isAvailable = isDeviceAvailable(address);
  bool isCalibrated = false;

  if (isAvailable) {
    // Device is present, try to read status
    Wire.requestFrom(address, (uint8_t)1);
    if (Wire.available()) {
      isCalibrated = Wire.read();
    }
  }

  // Check if state changed
  DeviceState& prevState = deviceStates[displayRow];
  bool availabilityChanged = (prevState.available != isAvailable);
  bool calibrationChanged = (prevState.calibrated != isCalibrated);

  if (availabilityChanged || calibrationChanged) {
    // Update stored state
    prevState.available = isAvailable;
    prevState.calibrated = isCalibrated;

    // Update LEDs
    updateStatusLEDs(displayRow, isAvailable, isCalibrated);

    // Print only on change
    if (!isAvailable) {
      Serial.printf("Device (0x%02X): OFFLINE\n", address);
    } else {
      Serial.printf("Device (0x%02X): ONLINE - Calibrated: %s\n", address,
                    isCalibrated ? "TRUE" : "FALSE");
    }
  }
}

bool isDeviceAvailable(uint8_t addr) {
  Wire.beginTransmission(addr);
  uint8_t error = Wire.endTransmission();
  return (error == 0);
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