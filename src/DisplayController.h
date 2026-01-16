#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>

#include "SystemResults.h"

#define COLOR_NEON_GREEN 0x07E0
#define COLOR_DARK_GREEN 0x0320
#define COLOR_PROGRESS_FILLED 0x3FEC
#define COLOR_PROGRESS_EMPTY 0x19E3
#define COLOR_WARNING_ORANGE 0xF9C6
#define COLOR_INTERCEPT_ORANGE 0xFD80

#define LABEL_LEFT_X 15
#define VALUE_RIGHT_X 306
#define SHIELDING_LEFT_X 25
#define SHIELDING_RIGHT_X 295

#define TARGET_DATE_Y 44
#define CURRENT_DATE_Y 72
#define LAST_DEPARTURE_Y 100
#define SHIELDING_MODULES_Y 184
#define SHIELDING_CALIBRATION_Y 218
#define SHIELDING_POWER_Y 252
#define SHIELDING_DETECTION_RISK_Y 285
#define CORE_POWER_Y 421
#define SERVICE_LINK_Y 449

class DisplayController {
  public:
    DisplayController();
    void begin(int deviceCount, uint8_t currentMonth, uint8_t currentDay, uint16_t currentYear,
               uint8_t lastMonth, uint8_t lastDay, uint16_t lastYear);
    void updateTargetDate(uint8_t month = 0, uint8_t day = 0, uint16_t year = 0);
    void updateCurrentDate(uint8_t month, uint8_t day, uint16_t year);
    void updateLastDeparture(uint8_t month, uint8_t day, uint16_t year);
    void updateShieldModules(int online, int calibrated);
    void updateInterceptWindow(int hours, int minutes, int seconds, bool hoursChanged = true,
                               bool minutesChanged = true);
    void updateCorePower(const String& status);
    void updateServiceLink(bool connected);

  private:
    TFT_eSPI tft;
    int totalDevices;
    void enableBacklight();
    void renderChrome();
    void renderDateLabels();
    void renderInterceptLabels();
    void renderSystemLabels();
    void renderShieldChrome();
    void renderShieldLabels();
    void renderModuleConnectionProgress(int count);
    void renderModuleCalibrationProgress(int count);
    void updateShieldPower(int calibrated, int total);
    void updateDetectionRisk(float percentage);

    void renderLabel(const String& text, int y);
    void renderShieldLabel(const String& text, int y);

    void renderValue(const String& text, int y, uint16_t color);
    void renderShieldValue(const String& text, int y, uint16_t color);
};
