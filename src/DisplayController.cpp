#include "DisplayController.h"

DisplayController::DisplayController() {
}

void DisplayController::begin(int deviceCount) {
  totalDevices = deviceCount;

  tft.init();
  tft.setRotation(0);  // 1 = landscape, 0 = portrait
  enableBacklight();
  tft.fillScreen(TFT_BLACK);

  const char* CorePowerValue_text = "ONLINE";
  const char* ServiceLinkValue_text = "DISCONNECTED";

  renderChrome();
  renderDateLabels();
  renderInterceptLabels();
  renderSystemLabels();

  renderShieldChrome();
  renderShieldLabels();

  updateDestinationDate();
  updateCurrentDate(1, 1, 2056);
  updateLastDeparture(12, 25, 2025);

  updateCorePower(CorePowerValue_text);
  updateServiceLink(false);

  // Initialize progress indicators with default state (0)
  renderModuleConnectionProgress(0);
  renderModuleCalibrationProgress(0);
}

void DisplayController::enableBacklight() {
  digitalWrite(TFT_BL, HIGH);
}

void DisplayController::renderChrome() {
  // Header Text
  tft.setTextColor(COLOR_NEON_GREEN);
  tft.setTextSize(1);
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.drawString("TIME DISPLACEMENT CONSOLE", 23, 7);
  // Header Line
  tft.drawLine(10, 30, 310, 30, COLOR_NEON_GREEN);
  // Bottom Line
  tft.drawLine(10, 410, 310, 410, COLOR_NEON_GREEN);
}

void DisplayController::renderDateLabels() {
  tft.setTextColor(COLOR_NEON_GREEN);
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.drawString("Destination Date", 15, 44);
  tft.drawString("Current Date", 15, 72);
  tft.drawString("Last Departure", 15, 100);
}

void DisplayController::renderInterceptLabels() {
  tft.setTextColor(COLOR_INTERCEPT_ORANGE);
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.drawString("INTERCEPT WINDOW", 72, 336);
}

void DisplayController::renderSystemLabels() {
  tft.setTextColor(COLOR_NEON_GREEN);
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.drawString("Core Power", 15, 421);
  tft.drawString("Service Link", 15, 449);
}

void DisplayController::renderShieldChrome() {
  tft.drawRoundRect(10, 149, 300, 174, 5, COLOR_NEON_GREEN);
  tft.fillRoundRect(86, 140, 149, 18, 6, COLOR_NEON_GREEN);
  tft.setTextColor(TFT_BLACK);
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.drawString("SHIELDING", 111, 141);
}

void DisplayController::renderShieldLabels() {
  tft.setTextColor(COLOR_NEON_GREEN);
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.drawString("Modules", 25, 184);
  tft.drawString("Calibrated", 25, 218);
  tft.drawString("Power", 25, 252);
  tft.drawString("Detection Risk", 25, 285);
}

void DisplayController::renderModuleConnectionProgress(int count) {
  const int xPositions[] = {180, 194, 208, 222, 236, 250, 264, 278};
  const int y = 181;
  const int width = 10;
  const int height = 20;

  for (int i = 0; i < 8; i++) {
    uint16_t color = (i < count) ? COLOR_PROGRESS_FILLED : COLOR_PROGRESS_EMPTY;
    tft.fillRect(xPositions[i], y, width, height, color);
  }
}

void DisplayController::renderModuleCalibrationProgress(int count) {
  const int xPositions[] = {180, 194, 208, 222, 236, 250, 264, 278};
  const int y = 214;
  const int width = 10;
  const int height = 20;

  for (int i = 0; i < 8; i++) {
    uint16_t color = (i < count) ? COLOR_PROGRESS_FILLED : COLOR_PROGRESS_EMPTY;
    tft.fillRect(xPositions[i], y, width, height, color);
  }
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
  tft.drawString(buf, 197, 44);
}

void DisplayController::updateCurrentDate(uint8_t month, uint8_t day, uint16_t year) {
  char buf[16];
  sprintf(buf, "%02d/%02d/%04d", month, day, year);

  // Clear old Value
  tft.fillRect(197, 72, 110, 15, TFT_BLACK);

  // Render new Value
  tft.setTextColor(TFT_WHITE);
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.drawString(buf, 197, 72);
}

void DisplayController::updateLastDeparture(uint8_t month, uint8_t day, uint16_t year) {
  char buf[16];
  sprintf(buf, "%02d/%02d/%04d", month, day, year);

  // Clear old Value
  tft.fillRect(197, 100, 110, 15, TFT_BLACK);

  // Render new Value
  tft.setTextColor(TFT_LIGHTGREY);
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.drawString(buf, 197, 100);
}

void DisplayController::updateCorePower(const String& status) {
  tft.setTextColor(TFT_WHITE);
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.drawString(status, 240, 421);
}

void DisplayController::updateShieldModules(int online, int calibrated) {
  renderModuleConnectionProgress(online);
  renderModuleCalibrationProgress(calibrated);
  updateShieldPower(calibrated, 8);
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
  tft.drawString(buf, 286, 252);
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
  tft.drawString(riskLevel, 286, 285);
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

void DisplayController::updateServiceLink(bool connected) {
  const char* status = connected ? "ESTABLISHED" : "DISCONNECTED";
  const uint16_t color = connected ? TFT_WHITE : COLOR_WARNING_ORANGE;

  // Clear old Value
  tft.fillRect(174, 451, 140, 15, TFT_BLACK);

  // Render new Value
  tft.setTextColor(color);
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.setTextDatum(TR_DATUM);
  tft.drawString(status, 306, 449);
  tft.setTextDatum(TL_DATUM);
}
