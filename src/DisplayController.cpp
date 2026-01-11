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

  updateDestinationDate();
  updateCurrentDate(1, 1, 2056);
  updateLastDeparture(12, 25, 2025);

  updateShieldModules(0, 0);

  updateCorePower("ONLINE");
  updateServiceLink(false);
}

void DisplayController::enableBacklight() {
  digitalWrite(TFT_BL, HIGH);
}

void DisplayController::renderChrome() {
  // Header Text
  tft.setTextColor(COLOR_NEON_GREEN);
  tft.setTextSize(1);
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("TIME DISPLACEMENT CONSOLE", 160, 7);
  tft.setTextDatum(TL_DATUM);
  // Header Line
  tft.drawLine(10, 30, 310, 30, COLOR_NEON_GREEN);
  // Bottom Line
  tft.drawLine(10, 410, 310, 410, COLOR_NEON_GREEN);
}

void DisplayController::renderDateLabels() {
  tft.setTextColor(COLOR_NEON_GREEN);
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.drawString("Destination Date", LABEL_LEFT_X, 44);
  tft.drawString("Current Date", LABEL_LEFT_X, 72);
  tft.drawString("Last Departure", LABEL_LEFT_X, 100);
}

void DisplayController::renderInterceptLabels() {
  tft.setTextColor(COLOR_INTERCEPT_ORANGE);
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("<<< INTERCEPT WINDOW >>>", 160, 336);
  tft.setTextDatum(TL_DATUM);
}

void DisplayController::renderSystemLabels() {
  tft.setTextColor(COLOR_NEON_GREEN);
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.drawString("Core Power", LABEL_LEFT_X, 421);
  tft.drawString("Service Link", LABEL_LEFT_X, 449);
}

void DisplayController::renderShieldChrome() {
  tft.drawRoundRect(10, 150, 300, 175, 5, COLOR_NEON_GREEN);
  tft.fillRoundRect(85, 140, 150, 20, 6, COLOR_NEON_GREEN);
  tft.setTextColor(TFT_BLACK);
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("SHIELDING", 160, 142);
  tft.setTextDatum(TL_DATUM);
}

void DisplayController::renderShieldLabels() {
  tft.setTextColor(COLOR_NEON_GREEN);
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.drawString("Modules", SHIELDING_LEFT_X, 184);
  tft.drawString("Calibrated", SHIELDING_LEFT_X, 218);
  tft.drawString("Power", SHIELDING_LEFT_X, 252);
  tft.drawString("Detection Risk", SHIELDING_LEFT_X, 285);
}

void DisplayController::updateDestinationDate(uint8_t month, uint8_t day, uint16_t year) {
  // Format the date string (MM/DD/YYYY or --/--/---- if no values provided)
  char buf[16];
  if (month == 0 && day == 0 && year == 0) {
    sprintf(buf, "--/--/----");
  } else {
    sprintf(buf, "%02d/%02d/%04d", month, day, year);
  }

  // Clear old Value
  tft.fillRect(197, 44, 110, 15, TFT_BLACK);

  // Render new Value
  tft.setTextColor(TFT_YELLOW);
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.setTextDatum(TR_DATUM);
  tft.drawString(buf, VALUE_RIGHT_X, 44);
  tft.setTextDatum(TL_DATUM);
}

void DisplayController::updateCurrentDate(uint8_t month, uint8_t day, uint16_t year) {
  char buf[16];
  sprintf(buf, "%02d/%02d/%04d", month, day, year);

  // Clear old Value
  tft.fillRect(197, 72, 110, 15, TFT_BLACK);

  // Render new Value
  tft.setTextColor(TFT_WHITE);
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.setTextDatum(TR_DATUM);
  tft.drawString(buf, VALUE_RIGHT_X, 72);
  tft.setTextDatum(TL_DATUM);
}

void DisplayController::updateLastDeparture(uint8_t month, uint8_t day, uint16_t year) {
  char buf[16];
  sprintf(buf, "%02d/%02d/%04d", month, day, year);

  // Clear old Value
  tft.fillRect(197, 100, 110, 15, TFT_BLACK);

  // Render new Value
  tft.setTextColor(TFT_LIGHTGREY);
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.setTextDatum(TR_DATUM);
  tft.drawString(buf, VALUE_RIGHT_X, 100);
  tft.setTextDatum(TL_DATUM);
}

void DisplayController::updateShieldModules(int online, int calibrated) {
  renderModuleConnectionProgress(online);
  renderModuleCalibrationProgress(calibrated);
  updateShieldPower(calibrated, 8);
}

void DisplayController::renderModuleConnectionProgress(int count) {
  const int xPositions[] = {187, 201, 215, 229, 243, 257, 271, 285};
  const int y = 181;
  const int width = 10;
  const int height = 20;

  for (int i = 0; i < 8; i++) {
    uint16_t color = (i < count) ? COLOR_PROGRESS_FILLED : COLOR_PROGRESS_EMPTY;
    tft.fillRect(xPositions[i], y, width, height, color);
  }
}

void DisplayController::renderModuleCalibrationProgress(int count) {
  const int xPositions[] = {187, 201, 215, 229, 243, 257, 271, 285};
  const int y = 214;
  const int width = 10;
  const int height = 20;

  for (int i = 0; i < 8; i++) {
    uint16_t color = (i < count) ? COLOR_PROGRESS_FILLED : COLOR_PROGRESS_EMPTY;
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
  tft.fillRect(200, 252, 100, 15, TFT_BLACK);

  // Render new Value
  tft.setTextColor(TFT_WHITE);
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.setTextDatum(TR_DATUM);
  tft.drawString(buf, SHIELDING_RIGHT_X, 252);
  tft.setTextDatum(TL_DATUM);

  // Update detection risk based on shield power
  updateDetectionRisk(percentage);
}

void DisplayController::updateDetectionRisk(float percentage) {
  // Determine risk level based on shield power (inverse relationship)
  const char* riskLevel;
  if (percentage < 12.5f) {
    riskLevel = "CRITICAL";
  } else if (percentage < 25.0f) {
    riskLevel = "SEVERE";
  } else if (percentage < 37.5f) {
    riskLevel = "HIGH";
  } else if (percentage < 50.0f) {
    riskLevel = "ELEVATED";
  } else if (percentage < 62.5f) {
    riskLevel = "MODERATE";
  } else if (percentage < 75.0f) {
    riskLevel = "GUARDED";
  } else if (percentage < 87.5f) {
    riskLevel = "LOW";
  } else if (percentage < 100.0f) {
    riskLevel = "MINIMAL";
  } else {
    riskLevel = "SECURE";
  }

  // Clear old Value
  tft.fillRect(200, 285, 100, 15, TFT_BLACK);

  // Render new Value
  tft.setTextColor(COLOR_WARNING_ORANGE);
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.setTextDatum(TR_DATUM);
  tft.drawString(riskLevel, SHIELDING_RIGHT_X, 285);
  tft.setTextDatum(TL_DATUM);
}

void DisplayController::updateInterceptWindow(int hours, int minutes, int seconds) {
  // Format the time string (HH:MM:SS)
  char buf[16];
  sprintf(buf, "%02d:%02d:%02d", hours, minutes, seconds);

  // Clear old Value
  tft.fillRect(48, 356, 224, 40, TFT_BLACK);

  // Render new Value
  tft.setTextColor(COLOR_INTERCEPT_ORANGE);
  tft.setFreeFont(&FreeMonoBold24pt7b);
  tft.drawString(buf, 48, 356);
}

void DisplayController::updateCorePower(const String& status) {
  tft.setTextColor(TFT_WHITE);
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.setTextDatum(TR_DATUM);
  tft.drawString(status, VALUE_RIGHT_X, 421);
  tft.setTextDatum(TL_DATUM);
}

void DisplayController::updateServiceLink(bool connected) {
  const char* status = connected ? "ESTABLISHED" : "DISCONNECTED";
  const uint16_t color = connected ? TFT_WHITE : COLOR_WARNING_ORANGE;

  // Clear old Value
  tft.fillRect(174, 451, 140, 15, TFT_BLACK);

  // Render new Value
  tft.setTextColor(color);
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.setTextDatum(TR_DATUM);
  tft.drawString(status, VALUE_RIGHT_X, 449);
  tft.setTextDatum(TL_DATUM);
}
