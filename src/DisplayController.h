#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>

#include "SystemResults.h"

// Color constants for progress indicators
#define COLOR_PROGRESS_FILLED 0x3FEC
#define COLOR_PROGRESS_EMPTY 0x19E3

class DisplayController {
  public:
    DisplayController();
    void begin(int deviceCount);
    void updateDestinationDate(uint8_t month, uint8_t day, uint16_t year);
    void updateCurrentDate(uint8_t month, uint8_t day, uint16_t year);
    void updateLastDeparture(uint8_t month, uint8_t day, uint16_t year);
    void updateCorePower(const String& status);
    void updateShieldModules(int online, int calibrated);
    void updateDetectionRisk(const String& level);
    void updateInterceptWindow(int hours, int minutes, int seconds);
    void updateServiceLink(bool connected);

  private:
    TFT_eSPI tft;
    int totalDevices;
    void enableBacklight();
    void renderModuleConnectionProgress(int count);
    void renderModuleCalibrationProgress(int count);
    void updateShieldPower(int calibrated, int total);
};
