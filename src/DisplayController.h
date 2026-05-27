#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>

#include "SystemResults.h"

#define COLOR_NEON_GREEN 0x07E0
#define COLOR_DARK_GREEN 0x0320

#define COLOR_NEON_BLUE 0x0E9F
#define COLOR_DARK_BLUE 0x9C7

#define COLOR_WHITE 0xFFFF
#define COLOR_LIGHTGREY 0xC618

#define COLOR_WARNING_ORANGE 0xF9C6
#define COLOR_INTERCEPT_ORANGE 0xFD80

#define COLOR_CONNECTION_EMPTY COLOR_DARK_BLUE
#define COLOR_CONNECTION_FILLED COLOR_NEON_BLUE

#define COLOR_CALIBRATION_EMPTY COLOR_DARK_BLUE
#define COLOR_CALIBRATION_FILLED COLOR_WHITE

#define HEADER_TEXT_COLOR COLOR_NEON_BLUE
#define CHROME_COLOR COLOR_NEON_BLUE

#define DATE_LABEL_COLOR COLOR_NEON_BLUE

#define SHIELDING_CHROME_COLOR COLOR_NEON_BLUE
#define SHIELDING_HEADER_OUTER_COLOR COLOR_DARK_BLUE
#define SHIELDING_HEADER_INNER_COLOR COLOR_NEON_BLUE
#define SHIELDING_LABEL_COLOR COLOR_NEON_BLUE

#define SHIELDING_CONNECTION_PROGRESS_EMPTY_COLOR COLOR_CONNECTION_EMPTY
#define SHIELDING_CONNECTION_PROGRESS_FILLED_COLOR COLOR_CONNECTION_FILLED
#define SHIELDING_CALIBRATION_PROGRESS_EMPTY_COLOR COLOR_CALIBRATION_EMPTY
#define SHIELDING_CALIBRATION_PROGRESS_FILLED_COLOR COLOR_CALIBRATION_FILLED

#define INTERCEPT_LABEL_COLOR COLOR_INTERCEPT_ORANGE
#define INTERCEPT_VALUE_COLOR COLOR_INTERCEPT_ORANGE

#define SYSTEM_LABEL_COLOR COLOR_NEON_BLUE

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
    void begin(int deviceCount);
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
    void updateDetectionRisk(int calibrated);

    void renderLabel(const String& text, int y);
    void renderShieldLabel(const String& text, int y);

    void renderValue(const String& text, int y, uint16_t color);
    void renderShieldValue(const String& text, int y, uint16_t color);
};
