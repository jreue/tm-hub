#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>

#include "SystemResults.h"

class DisplayController {
  public:
    DisplayController();
    void begin(int deviceCount);
    void updateDestinationDate(uint8_t month, uint8_t day, uint16_t year);
    void updateCurrentDate(uint8_t month, uint8_t day, uint16_t year);
    void updateLastDeparture(uint8_t month, uint8_t day, uint16_t year);
    void updateCorePower(const String& status);
    void updateShieldModules(int online, int calibrated);
    void updateShieldStatus(const String& status);
    void updateDetectionRisk(const String& level);
    void updateInterceptWindow(int hours, int minutes, int seconds);
    void updateServiceLink(bool connected);

  private:
    TFT_eSPI tft;
    int totalDevices;
    void enableBacklight();
    void renderDateResults(int32_t startY, int32_t lineHeight);
    void renderModuleResults(int32_t startY, int32_t lineHeight);
    void renderRiskResults(int32_t startY, int32_t lineHeight);
    void renderSystemResults(int32_t startY, int32_t lineHeight);
    void renderSystemResultItem(const char* label, const char* value, uint16_t valueColor,
                                int32_t y);
};
