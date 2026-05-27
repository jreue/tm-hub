#include "DisplayController.h"

DisplayController::DisplayController() {
}

void DisplayController::begin(int deviceCount) {
  totalDevices = deviceCount;

  tft.init();
  tft.setRotation(0);  // 1 = landscape, 0 = portrait
  enableBacklight();
  tft.fillScreen(TFT_BLACK);

  renderChrome();
  renderDateLabels();
  renderInterceptLabels();
  renderSystemLabels();

  renderShieldChrome();
  renderShieldLabels();

  updateTargetDate(0, 0, 0);
  updateCurrentDate(0, 0, 0);
  updateLastDeparture(0, 0, 0);

  updateShieldModules(0, 0);

  updateCorePower("ONLINE");
  updateServiceLink(false);
}

void DisplayController::enableBacklight() {
  digitalWrite(TFT_BL, HIGH);
}

void DisplayController::renderLabel(const String& text, int y) {
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.drawString(text, LABEL_LEFT_X, y);
}

void DisplayController::renderShieldLabel(const String& text, int y) {
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.drawString(text, SHIELDING_LEFT_X, y);
}

void DisplayController::renderValue(const String& text, int y, uint16_t color) {
  tft.setTextColor(color);
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.setTextDatum(TR_DATUM);
  tft.drawString(text, VALUE_RIGHT_X, y);
  tft.setTextDatum(TL_DATUM);
}

void DisplayController::renderShieldValue(const String& text, int y, uint16_t color) {
  tft.setTextColor(color);
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.setTextDatum(TR_DATUM);
  tft.drawString(text, SHIELDING_RIGHT_X, y);
  tft.setTextDatum(TL_DATUM);
}

void DisplayController::renderChrome() {
  // Header Text
  tft.setTextColor(HEADER_TEXT_COLOR);
  tft.setTextSize(1);
  tft.setFreeFont(&FreeSansBold9pt7b);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("TIME DISPLACEMENT MONITOR", 160, 7);
  tft.setTextDatum(TL_DATUM);
  // Header Line
  tft.drawLine(10, 30, 310, 30, CHROME_COLOR);
  // Bottom Line
  tft.drawLine(10, 410, 310, 410, CHROME_COLOR);
}

void DisplayController::renderDateLabels() {
  tft.setTextColor(DATE_LABEL_COLOR);

  renderLabel("Target Date", TARGET_DATE_Y);
  renderLabel("Current Date", CURRENT_DATE_Y);
  renderLabel("Last Departure", LAST_DEPARTURE_Y);
}

void DisplayController::renderInterceptLabels() {
  tft.setTextColor(INTERCEPT_LABEL_COLOR);

  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("<<< INTERCEPT WINDOW >>>", 160, 336);
  tft.setTextDatum(TL_DATUM);

  tft.setTextColor(INTERCEPT_VALUE_COLOR);
  tft.setFreeFont(&FreeMonoBold24pt7b);
  tft.drawString(":", 104, 352);
  tft.drawString(":", 188, 352);
}

void DisplayController::renderSystemLabels() {
  tft.setTextColor(SYSTEM_LABEL_COLOR);

  renderLabel("Core Power", CORE_POWER_Y);
  renderLabel("Service Link", SERVICE_LINK_Y);
}

void DisplayController::renderShieldChrome() {
  // Outer border
  tft.drawRoundRect(10, 150, 300, 175, 5, SHIELDING_CHROME_COLOR);

  // Title background
  tft.fillRoundRect(83, 138, 154, 24, 7, SHIELDING_HEADER_OUTER_COLOR);
  tft.fillRoundRect(85, 140, 150, 20, 6, SHIELDING_HEADER_INNER_COLOR);

  tft.setTextColor(TFT_BLACK);
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("SHIELDING", 160, 142);
  tft.setTextDatum(TL_DATUM);
}

void DisplayController::renderShieldLabels() {
  tft.setTextColor(SHIELDING_LABEL_COLOR);

  renderShieldLabel("Modules", SHIELDING_MODULES_Y);
  renderShieldLabel("Calibrated", SHIELDING_CALIBRATION_Y);
  renderShieldLabel("Power", SHIELDING_POWER_Y);
  renderShieldLabel("Detection Risk", SHIELDING_DETECTION_RISK_Y);
}

void DisplayController::updateTargetDate(uint8_t month, uint8_t day, uint16_t year) {
  // Format the date string (MM/DD/YYYY or --/--/---- if no values provided)
  char buf[16];
  if (month == 0 && day == 0 && year == 0) {
    sprintf(buf, "--/--/----");
  } else {
    sprintf(buf, "%02d/%02d/%04d", month, day, year);
  }

  // Clear old Value
  tft.fillRect(197, TARGET_DATE_Y, 110, 15, TFT_BLACK);
  // Render new Value
  renderValue(buf, TARGET_DATE_Y, TFT_YELLOW);
}

void DisplayController::updateCurrentDate(uint8_t month, uint8_t day, uint16_t year) {
  char buf[16];
  if (month == 0 && day == 0 && year == 0) {
    sprintf(buf, "--/--/----");
  } else {
    sprintf(buf, "%02d/%02d/%04d", month, day, year);
  }

  // Clear old Value
  tft.fillRect(197, CURRENT_DATE_Y, 110, 15, TFT_BLACK);
  // Render new Value
  renderValue(buf, CURRENT_DATE_Y, TFT_WHITE);
}

void DisplayController::updateLastDeparture(uint8_t month, uint8_t day, uint16_t year) {
  char buf[16];
  if (month == 0 && day == 0 && year == 0) {
    sprintf(buf, "--/--/----");
  } else {
    sprintf(buf, "%02d/%02d/%04d", month, day, year);
  }

  // Clear old Value
  tft.fillRect(197, LAST_DEPARTURE_Y, 110, 15, TFT_BLACK);
  // Render new Value
  renderValue(buf, LAST_DEPARTURE_Y, TFT_LIGHTGREY);
}

void DisplayController::updateShieldModules(int online, int calibrated) {
  renderModuleConnectionProgress(online);
  renderModuleCalibrationProgress(calibrated);
  updateShieldPower(calibrated, totalDevices);
}

void DisplayController::renderModuleConnectionProgress(int count) {
  const int xPositions[] = {215, 229, 243, 257, 271, 285};
  const int y = 180;
  const int width = 10;
  const int height = 20;

  for (int i = 0; i < totalDevices; i++) {
    uint16_t color = (i < count) ? SHIELDING_CONNECTION_PROGRESS_FILLED_COLOR
                                 : SHIELDING_CONNECTION_PROGRESS_EMPTY_COLOR;
    tft.fillRect(xPositions[i], y, width, height, color);
  }
}

void DisplayController::renderModuleCalibrationProgress(int count) {
  const int xPositions[] = {215, 229, 243, 257, 271, 285};
  const int y = 215;
  const int width = 10;
  const int height = 20;

  for (int i = 0; i < totalDevices; i++) {
    uint16_t color = (i < count) ? SHIELDING_CALIBRATION_PROGRESS_FILLED_COLOR
                                 : SHIELDING_CALIBRATION_PROGRESS_EMPTY_COLOR;
    tft.fillRect(xPositions[i], y, width, height, color);
  }
}

void DisplayController::updateShieldPower(int calibrated, int total) {
  // Calculate percentage with decimal precision
  float percentage = (total > 0) ? (calibrated * 100.0f) / total : 0.0f;

  // Format the percentage string with 1 decimal place
  char buf[10];
  sprintf(buf, "%.1f%%", percentage);

  // Clear old Value
  tft.fillRect(200, SHIELDING_POWER_Y, 100, 15, TFT_BLACK);
  // Render new Value
  renderShieldValue(buf, SHIELDING_POWER_Y, TFT_WHITE);

  // Update detection risk based on shield power
  updateDetectionRisk(calibrated);
}

void DisplayController::updateDetectionRisk(int calibrated) {
  const char* riskLevel;
  switch (calibrated) {
    case 0:
      riskLevel = "CRITICAL";
      break;
    case 1:
      riskLevel = "SEVERE";
      break;
    case 2:
      riskLevel = "ELEVATED";
      break;
    case 3:
      riskLevel = "MODERATE";
      break;
    case 4:
      riskLevel = "LOW";
      break;
    case 5:
      riskLevel = "MINIMAL";
      break;
    default:
      riskLevel = "SECURE";
      break;
  }

  // Clear old Value
  tft.fillRect(200, SHIELDING_DETECTION_RISK_Y, 100, 15, TFT_BLACK);
  // Render new Value
  renderShieldValue(riskLevel, SHIELDING_DETECTION_RISK_Y, COLOR_WARNING_ORANGE);
}

void DisplayController::updateInterceptWindow(int hours, int minutes, int seconds,
                                              bool hoursChanged, bool minutesChanged) {
  tft.setTextColor(INTERCEPT_VALUE_COLOR);
  tft.setFreeFont(&FreeMonoBold24pt7b);

  // Only update the parts that changed to avoid flicker
  char buf[4];

  if (hoursChanged) {
    sprintf(buf, "%02d", hours);
    tft.fillRect(48, 356, 56, 40, TFT_BLACK);  // Clear hours section
    tft.drawString(buf, 48, 356);
  }

  if (minutesChanged) {
    sprintf(buf, "%02d", minutes);
    tft.fillRect(132, 356, 56, 40, TFT_BLACK);  // Clear minutes section
    tft.drawString(buf, 132, 356);
  }

  // Seconds always change
  sprintf(buf, "%02d", seconds);
  tft.fillRect(216, 356, 56, 40, TFT_BLACK);  // Clear seconds section
  tft.drawString(buf, 216, 356);
}

void DisplayController::updateCorePower(const String& status) {
  renderValue(status, CORE_POWER_Y, TFT_WHITE);
}

void DisplayController::updateServiceLink(bool connected) {
  const char* status = connected ? "ESTABLISHED" : "DISCONNECTED";
  const uint16_t color = connected ? TFT_WHITE : COLOR_WARNING_ORANGE;

  // Clear old Value
  tft.fillRect(174, SERVICE_LINK_Y, 140, 15, TFT_BLACK);
  // Render new Value
  renderValue(status, SERVICE_LINK_Y, color);
}
