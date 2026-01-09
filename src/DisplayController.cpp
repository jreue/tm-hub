#include "DisplayController.h"

DisplayController::DisplayController() {
}

void DisplayController::begin(int deviceCount) {
  totalDevices = deviceCount;
  // TFT Initialization
  tft.init();
  tft.setRotation(0);  // 1 = landscape, 0 = portrait
  enableBacklight();
  tft.fillScreen(TFT_BLACK);

  // Draw title
  tft.setFreeFont(&FreeSansBold9pt7b);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("TIME DISPLACEMENT CONSOLE", 10, 10);
  tft.drawLine(10, 25, tft.width() - 10, 25, TFT_WHITE);

  // Initialize Shield Modules value with device count
  SystemResults::moduleResults[SystemResults::IDX_MODULE_SHIELD_MODULES].value =
      "0 / " + String(totalDevices);
  SystemResults::moduleResults[SystemResults::IDX_MODULE_SHIELD_CALIBRATED].value =
      "0 / " + String(totalDevices);

  // Render all groups with different positioning
  int32_t currentY = 35;
  renderDateResults(currentY, 25);
  currentY += SystemResults::dateResultCount * 25 + 10;  // Add spacing between groups

  renderModuleResults(currentY, 25);
  currentY += SystemResults::moduleResultCount * 25 + 10;

  renderRiskResults(currentY, 25);

  // Render system results at bottom of screen
  // FreeSansBold9pt7b needs ~20px clearance above baseline, so start at 450
  renderSystemResults(430, 25);
}

void DisplayController::enableBacklight() {
  digitalWrite(TFT_BL, HIGH);
}

void DisplayController::renderDateResults(int32_t startY, int32_t lineHeight) {
  for (size_t i = 0; i < SystemResults::dateResultCount; ++i) {
    renderSystemResultItem(SystemResults::dateResults[i].label.c_str(),
                           SystemResults::dateResults[i].value.c_str(),
                           SystemResults::dateResults[i].color, startY + i * lineHeight);
  }
}

void DisplayController::renderModuleResults(int32_t startY, int32_t lineHeight) {
  for (size_t i = 0; i < SystemResults::moduleResultCount; ++i) {
    renderSystemResultItem(SystemResults::moduleResults[i].label.c_str(),
                           SystemResults::moduleResults[i].value.c_str(),
                           SystemResults::moduleResults[i].color, startY + i * lineHeight);
  }
}

void DisplayController::renderRiskResults(int32_t startY, int32_t lineHeight) {
  for (size_t i = 0; i < SystemResults::riskResultCount; ++i) {
    renderSystemResultItem(SystemResults::riskResults[i].label.c_str(),
                           SystemResults::riskResults[i].value.c_str(),
                           SystemResults::riskResults[i].color, startY + i * lineHeight);
  }
}

void DisplayController::renderSystemResults(int32_t startY, int32_t lineHeight) {
  for (size_t i = 0; i < SystemResults::systemResultCount; ++i) {
    renderSystemResultItem(SystemResults::systemResults[i].label.c_str(),
                           SystemResults::systemResults[i].value.c_str(),
                           SystemResults::systemResults[i].color, startY + i * lineHeight);
  }
}

void DisplayController::renderSystemResultItem(const char* label, const char* value,
                                               uint16_t valueColor, int32_t y) {
  // Clear the line
  tft.fillRect(10, y, tft.width() - 20, 20, TFT_BLACK);

  tft.setFreeFont(&FreeSansBold9pt7b);
  // Draw label (left-aligned)
  tft.setTextColor(TFT_CYAN);
  tft.drawString(label + String(":"), 10, y);

  // Draw value (right-aligned)
  tft.setTextColor(valueColor);
  int labelWidth = tft.textWidth(label + String(":"));
  tft.drawString(value, 20 + labelWidth, y);
}

void DisplayController::updateDestinationDate(uint8_t month, uint8_t day, uint16_t year) {
  char buf[16];
  sprintf(buf, "%02d / %02d / %04d", month, day, year);
  SystemResults::dateResults[SystemResults::IDX_DATE_DESTINATION].value = buf;
  int32_t y = 35 + SystemResults::IDX_DATE_DESTINATION * 25;
  renderSystemResultItem(
      SystemResults::dateResults[SystemResults::IDX_DATE_DESTINATION].label.c_str(),
      SystemResults::dateResults[SystemResults::IDX_DATE_DESTINATION].value.c_str(),
      SystemResults::dateResults[SystemResults::IDX_DATE_DESTINATION].color, y);
}

void DisplayController::updateCurrentDate(uint8_t month, uint8_t day, uint16_t year) {
  char buf[16];
  sprintf(buf, "%02d / %02d / %04d", month, day, year);
  SystemResults::dateResults[SystemResults::IDX_DATE_CURRENT].value = buf;
  int32_t y = 35 + SystemResults::IDX_DATE_CURRENT * 25;
  renderSystemResultItem(SystemResults::dateResults[SystemResults::IDX_DATE_CURRENT].label.c_str(),
                         SystemResults::dateResults[SystemResults::IDX_DATE_CURRENT].value.c_str(),
                         SystemResults::dateResults[SystemResults::IDX_DATE_CURRENT].color, y);
}

void DisplayController::updateLastDeparture(uint8_t month, uint8_t day, uint16_t year) {
  char buf[16];
  sprintf(buf, "%02d / %02d / %04d", month, day, year);
  SystemResults::dateResults[SystemResults::IDX_DATE_LAST_DEPARTURE].value = buf;
  int32_t y = 35 + SystemResults::IDX_DATE_LAST_DEPARTURE * 25;
  renderSystemResultItem(
      SystemResults::dateResults[SystemResults::IDX_DATE_LAST_DEPARTURE].label.c_str(),
      SystemResults::dateResults[SystemResults::IDX_DATE_LAST_DEPARTURE].value.c_str(),
      SystemResults::dateResults[SystemResults::IDX_DATE_LAST_DEPARTURE].color, y);
}

void DisplayController::updateCorePower(const String& status) {
  SystemResults::systemResults[SystemResults::IDX_SYSTEM_CORE_POWER].value = status;
  int32_t y = 450 + SystemResults::IDX_SYSTEM_CORE_POWER * 25;
  renderSystemResultItem(
      SystemResults::systemResults[SystemResults::IDX_SYSTEM_CORE_POWER].label.c_str(),
      SystemResults::systemResults[SystemResults::IDX_SYSTEM_CORE_POWER].value.c_str(),
      SystemResults::systemResults[SystemResults::IDX_SYSTEM_CORE_POWER].color, y);
}

void DisplayController::updateShieldModules(int online, int calibrated) {
  SystemResults::moduleResults[SystemResults::IDX_MODULE_SHIELD_MODULES].value =
      String(online) + " / " + String(totalDevices);
  SystemResults::moduleResults[SystemResults::IDX_MODULE_SHIELD_CALIBRATED].value =
      String(calibrated) + " / " + String(totalDevices);

  // Update shield status based on calibration state
  String status;
  if (online == 0) {
    status = "OFFLINE";
  } else if (calibrated == 0) {
    status = "AVAILABLE";
  } else {
    // Calculate percentage of calibrated devices out of total
    int percentage = (calibrated * 100) / totalDevices;
    status = String(percentage) + "% Active";
  }
  SystemResults::moduleResults[SystemResults::IDX_MODULE_SHIELD_STATUS].value = status;

  // Calculate Y position for module group
  int32_t moduleStartY = 35 + SystemResults::dateResultCount * 25 + 10;

  // Render all three lines
  renderSystemResultItem(
      SystemResults::moduleResults[SystemResults::IDX_MODULE_SHIELD_MODULES].label.c_str(),
      SystemResults::moduleResults[SystemResults::IDX_MODULE_SHIELD_MODULES].value.c_str(),
      SystemResults::moduleResults[SystemResults::IDX_MODULE_SHIELD_MODULES].color,
      moduleStartY + SystemResults::IDX_MODULE_SHIELD_MODULES * 25);
  renderSystemResultItem(
      SystemResults::moduleResults[SystemResults::IDX_MODULE_SHIELD_CALIBRATED].label.c_str(),
      SystemResults::moduleResults[SystemResults::IDX_MODULE_SHIELD_CALIBRATED].value.c_str(),
      SystemResults::moduleResults[SystemResults::IDX_MODULE_SHIELD_CALIBRATED].color,
      moduleStartY + SystemResults::IDX_MODULE_SHIELD_CALIBRATED * 25);
  renderSystemResultItem(
      SystemResults::moduleResults[SystemResults::IDX_MODULE_SHIELD_STATUS].label.c_str(),
      SystemResults::moduleResults[SystemResults::IDX_MODULE_SHIELD_STATUS].value.c_str(),
      SystemResults::moduleResults[SystemResults::IDX_MODULE_SHIELD_STATUS].color,
      moduleStartY + SystemResults::IDX_MODULE_SHIELD_STATUS * 25);
}

void DisplayController::updateShieldStatus(const String& status) {
  SystemResults::moduleResults[SystemResults::IDX_MODULE_SHIELD_STATUS].value = status;
  int32_t moduleStartY = 35 + SystemResults::dateResultCount * 25 + 10;
  int32_t y = moduleStartY + SystemResults::IDX_MODULE_SHIELD_STATUS * 25;
  renderSystemResultItem(
      SystemResults::moduleResults[SystemResults::IDX_MODULE_SHIELD_STATUS].label.c_str(),
      SystemResults::moduleResults[SystemResults::IDX_MODULE_SHIELD_STATUS].value.c_str(),
      SystemResults::moduleResults[SystemResults::IDX_MODULE_SHIELD_STATUS].color, y);
}

void DisplayController::updateDetectionRisk(const String& level) {
  SystemResults::riskResults[SystemResults::IDX_RISK_DETECTION_RISK].value = level;
  int32_t riskStartY =
      35 + (SystemResults::dateResultCount + SystemResults::moduleResultCount) * 25 + 20;
  int32_t y = riskStartY + SystemResults::IDX_RISK_DETECTION_RISK * 25;
  renderSystemResultItem(
      SystemResults::riskResults[SystemResults::IDX_RISK_DETECTION_RISK].label.c_str(),
      SystemResults::riskResults[SystemResults::IDX_RISK_DETECTION_RISK].value.c_str(),
      SystemResults::riskResults[SystemResults::IDX_RISK_DETECTION_RISK].color, y);
}

void DisplayController::updateInterceptWindow(int hours, int minutes, int seconds) {
  char buf[16];
  sprintf(buf, "%02dh %02dm %02ds", hours, minutes, seconds);
  SystemResults::riskResults[SystemResults::IDX_RISK_INTERCEPT_WINDOW].value = buf;
  int32_t riskStartY =
      35 + (SystemResults::dateResultCount + SystemResults::moduleResultCount) * 25 + 20;
  int32_t y = riskStartY + SystemResults::IDX_RISK_INTERCEPT_WINDOW * 25;
  renderSystemResultItem(
      SystemResults::riskResults[SystemResults::IDX_RISK_INTERCEPT_WINDOW].label.c_str(),
      SystemResults::riskResults[SystemResults::IDX_RISK_INTERCEPT_WINDOW].value.c_str(),
      SystemResults::riskResults[SystemResults::IDX_RISK_INTERCEPT_WINDOW].color, y);
}

void DisplayController::updateServiceLink(bool connected) {
  SystemResults::systemResults[SystemResults::IDX_SYSTEM_SERVICE_LINK].value =
      connected ? "CONNECTED" : "DISCONNECTED";
  int32_t y = 450 + SystemResults::IDX_SYSTEM_SERVICE_LINK * 25;
  renderSystemResultItem(
      SystemResults::systemResults[SystemResults::IDX_SYSTEM_SERVICE_LINK].label.c_str(),
      SystemResults::systemResults[SystemResults::IDX_SYSTEM_SERVICE_LINK].value.c_str(),
      SystemResults::systemResults[SystemResults::IDX_SYSTEM_SERVICE_LINK].color, y);
}
