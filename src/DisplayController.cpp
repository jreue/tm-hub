#include "DisplayController.h"

DisplayController::DisplayController() {
}

void DisplayController::begin(int deviceCount) {
  totalDevices = deviceCount;

  tft.init();
  tft.setRotation(0);  // 1 = landscape, 0 = portrait
  enableBacklight();
  tft.fillScreen(TFT_BLACK);

  const char* DestinationValue_text = "--/--/--";
  const char* CurrentDateValue_text = "06/04/2056";
  const char* LastDepartureValue_text = "12/25/2025";
  const char* CorePowerValue_text = "ONLINE";
  const char* ServiceLinkValue_text = "DISCONNECTED";

  // Header Text
  tft.setTextColor(COLOR_NEON_GREEN);
  tft.setTextSize(1);
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.drawString("TIME DISPLACEMENT CONSOLE", 23, 7);
  // Header Underline
  tft.drawLine(10, 30, 310, 30, COLOR_NEON_GREEN);
  // Core Power Label
  tft.drawString("Core Power", 15, 421);
  // Service Link Label
  tft.drawString("Service Link", 15, 449);
  // Bottom Line
  tft.drawLine(10, 410, 310, 410, COLOR_NEON_GREEN);
  // Intercept Window Label
  tft.setTextColor(COLOR_INTERCEPT_ORANGE);
  tft.drawString("INTERCEPT WINDOW", 72, 336);
  // Intercept Time
  tft.setFreeFont(&FreeMonoBold24pt7b);
  tft.drawString("--:--:--", 48, 356);
  // Destination Label
  tft.setTextColor(COLOR_NEON_GREEN);
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.drawString("Destination Date", 15, 44);
  // Current Label
  tft.drawString("Current Date", 15, 72);
  // Last Departure Label
  tft.drawString("Last Departure", 15, 100);
  // Shield Border
  tft.drawRoundRect(10, 149, 300, 174, 5, COLOR_NEON_GREEN);
  // Shield Container
  tft.fillRoundRect(86, 140, 149, 18, 6, COLOR_NEON_GREEN);
  // Shielding Header
  tft.setTextColor(TFT_BLACK);
  tft.drawString("SHIELDING", 111, 141);
  // Modules Label
  tft.setTextColor(COLOR_NEON_GREEN);
  tft.drawString("Modules", 25, 184);
  // Calibrated Label
  tft.drawString("Calibrated", 25, 218);
  // Power Label
  tft.drawString("Power", 25, 252);
  // Risk Label
  tft.drawString("Detection Risk", 25, 285);

  // DestinationValue
  tft.setTextColor(TFT_YELLOW);
  tft.drawString(DestinationValue_text, 219, 44);
  // CurrentDateValue
  tft.setTextColor(TFT_WHITE);
  tft.drawString(CurrentDateValue_text, 197, 72);
  // LastDepartureValue
  tft.setTextColor(TFT_BLUE);
  tft.drawString(LastDepartureValue_text, 196, 100);

  // CorePowerValue
  tft.setTextColor(TFT_WHITE);
  tft.drawString(CorePowerValue_text, 240, 421);

  // ServiceLinkValue
  tft.setTextColor(COLOR_WARNING_ORANGE);
  tft.drawString(ServiceLinkValue_text, 174, 449);

  // Initialize progress indicators with default state (0)
  renderModuleConnectionProgress(0);
  renderModuleCalibrationProgress(0);
}

void DisplayController::enableBacklight() {
  digitalWrite(TFT_BL, HIGH);
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
  // Format the date string (MM/DD/YYYY)
  char buf[16];
  sprintf(buf, "%02d/%02d/%04d", month, day, year);

  // Clear the area where the date will be drawn
  // Using approximate width based on FreeMonoBold9pt7b font (~90 pixels for date string)
  tft.fillRect(197, 44, 110, 15, TFT_BLACK);

  // Draw the new date value
  tft.setTextColor(TFT_YELLOW);
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.drawString(buf, 197, 44);
}

void DisplayController::updateCurrentDate(uint8_t month, uint8_t day, uint16_t year) {
  // Format the date string (MM/DD/YYYY)
  char buf[16];
  sprintf(buf, "%02d/%02d/%04d", month, day, year);

  // Clear the area where the date will be drawn
  // Using approximate width based on FreeMonoBold9pt7b font (~90 pixels for date string)
  tft.fillRect(197, 72, 110, 15, TFT_BLACK);

  // Draw the new date value
  tft.setTextColor(TFT_WHITE);
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.drawString(buf, 197, 72);
}

void DisplayController::updateLastDeparture(uint8_t month, uint8_t day, uint16_t year) {
  // Format the date string (MM/DD/YYYY)
  char buf[16];
  sprintf(buf, "%02d/%02d/%04d", month, day, year);

  // Clear the area where the date will be drawn
  // Using approximate width based on FreeMonoBold9pt7b font (~90 pixels for date string)
  tft.fillRect(197, 100, 110, 15, TFT_BLACK);

  // Draw the new date value
  tft.setTextColor(TFT_BLUE);
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.drawString(buf, 197, 100);
}

void DisplayController::updateCorePower(const String& status) {
  // Stub - implementation removed
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

  // Clear the area where the power percentage will be drawn
  tft.fillRect(200, 252, 100, 15, TFT_BLACK);

  // Draw the new power percentage
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

  // Clear the area where the risk level will be drawn
  tft.fillRect(200, 285, 100, 15, TFT_BLACK);

  // Draw the new risk level
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

  // Clear the area where the intercept window will be drawn
  tft.fillRect(48, 356, 224, 40, TFT_BLACK);

  // Draw the new intercept window time
  tft.setTextColor(COLOR_INTERCEPT_ORANGE);
  tft.setFreeFont(&FreeMonoBold24pt7b);
  tft.drawString(buf, 48, 356);
}

void DisplayController::updateServiceLink(bool connected) {
  const char* status = connected ? "ESTABLISHED" : "DISCONNECTED";
  const uint16_t color = connected ? TFT_WHITE : COLOR_WARNING_ORANGE;
  // Clear the area where the service link text will be drawn
  tft.fillRect(174, 451, 140, 15, TFT_BLACK);

  // Draw the new service link status
  tft.setTextColor(color);
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.setTextDatum(TR_DATUM);
  tft.drawString(status, 306, 449);
  tft.setTextDatum(TL_DATUM);
}
