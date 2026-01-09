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
  SystemResults::results[SystemResults::IDX_SHIELD_MODULES].value = "0 / " + String(totalDevices);
  SystemResults::results[SystemResults::IDX_SHIELD_MODULES_CALIBRATED].value =
      "0 / " + String(totalDevices);

  // Render all system results
  renderSystemResults();
}

void DisplayController::enableBacklight() {
  digitalWrite(TFT_BL, HIGH);
}

void DisplayController::renderSystemResults() {
  const SystemStatus* results = SystemResults::getResults();
  size_t count = SystemResults::getResultCount();
  int32_t startY = 35;
  int32_t lineHeight = 25;

  for (size_t i = 0; i < count; ++i) {
    renderSystemResultItem(results[i].label.c_str(), results[i].value.c_str(), results[i].color,
                           startY + i * lineHeight);
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
  SystemResults::results[SystemResults::IDX_DESTINATION_DATE].value = buf;
  renderSystemResultItem(SystemResults::results[SystemResults::IDX_DESTINATION_DATE].label.c_str(),
                         SystemResults::results[SystemResults::IDX_DESTINATION_DATE].value.c_str(),
                         SystemResults::results[SystemResults::IDX_DESTINATION_DATE].color,
                         35 + SystemResults::IDX_DESTINATION_DATE * 25);
}

void DisplayController::updateCurrentDate(uint8_t month, uint8_t day, uint16_t year) {
  char buf[16];
  sprintf(buf, "%02d / %02d / %04d", month, day, year);
  SystemResults::results[SystemResults::IDX_CURRENT_DATE].value = buf;
  renderSystemResultItem(SystemResults::results[SystemResults::IDX_CURRENT_DATE].label.c_str(),
                         SystemResults::results[SystemResults::IDX_CURRENT_DATE].value.c_str(),
                         SystemResults::results[SystemResults::IDX_CURRENT_DATE].color,
                         35 + SystemResults::IDX_CURRENT_DATE * 25);
}

void DisplayController::updateLastDeparture(uint8_t month, uint8_t day, uint16_t year) {
  char buf[16];
  sprintf(buf, "%02d / %02d / %04d", month, day, year);
  SystemResults::results[SystemResults::IDX_LAST_DEPARTURE].value = buf;
  renderSystemResultItem(SystemResults::results[SystemResults::IDX_LAST_DEPARTURE].label.c_str(),
                         SystemResults::results[SystemResults::IDX_LAST_DEPARTURE].value.c_str(),
                         SystemResults::results[SystemResults::IDX_LAST_DEPARTURE].color,
                         35 + SystemResults::IDX_LAST_DEPARTURE * 25);
}

void DisplayController::updateCorePower(const String& status) {
  SystemResults::results[SystemResults::IDX_CORE_POWER].value = status;
  renderSystemResultItem(SystemResults::results[SystemResults::IDX_CORE_POWER].label.c_str(),
                         SystemResults::results[SystemResults::IDX_CORE_POWER].value.c_str(),
                         SystemResults::results[SystemResults::IDX_CORE_POWER].color,
                         35 + SystemResults::IDX_CORE_POWER * 25);
}

void DisplayController::updateShieldModules(int online, int calibrated) {
  SystemResults::results[SystemResults::IDX_SHIELD_MODULES].value =
      String(online) + " / " + String(totalDevices);
  SystemResults::results[SystemResults::IDX_SHIELD_MODULES_CALIBRATED].value =
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
  SystemResults::results[SystemResults::IDX_SHIELD_STATUS].value = status;

  // Render all three lines
  renderSystemResultItem(SystemResults::results[SystemResults::IDX_SHIELD_MODULES].label.c_str(),
                         SystemResults::results[SystemResults::IDX_SHIELD_MODULES].value.c_str(),
                         SystemResults::results[SystemResults::IDX_SHIELD_MODULES].color,
                         35 + SystemResults::IDX_SHIELD_MODULES * 25);
  renderSystemResultItem(
      SystemResults::results[SystemResults::IDX_SHIELD_MODULES_CALIBRATED].label.c_str(),
      SystemResults::results[SystemResults::IDX_SHIELD_MODULES_CALIBRATED].value.c_str(),
      SystemResults::results[SystemResults::IDX_SHIELD_MODULES_CALIBRATED].color,
      35 + SystemResults::IDX_SHIELD_MODULES_CALIBRATED * 25);
  renderSystemResultItem(SystemResults::results[SystemResults::IDX_SHIELD_STATUS].label.c_str(),
                         SystemResults::results[SystemResults::IDX_SHIELD_STATUS].value.c_str(),
                         SystemResults::results[SystemResults::IDX_SHIELD_STATUS].color,
                         35 + SystemResults::IDX_SHIELD_STATUS * 25);
}

void DisplayController::updateShieldStatus(const String& status) {
  SystemResults::results[SystemResults::IDX_SHIELD_STATUS].value = status;
  renderSystemResultItem(SystemResults::results[SystemResults::IDX_SHIELD_STATUS].label.c_str(),
                         SystemResults::results[SystemResults::IDX_SHIELD_STATUS].value.c_str(),
                         SystemResults::results[SystemResults::IDX_SHIELD_STATUS].color,
                         35 + SystemResults::IDX_SHIELD_STATUS * 25);
}

void DisplayController::updateDetectionRisk(const String& level) {
  SystemResults::results[SystemResults::IDX_DETECTION_RISK].value = level;
  renderSystemResultItem(SystemResults::results[SystemResults::IDX_DETECTION_RISK].label.c_str(),
                         SystemResults::results[SystemResults::IDX_DETECTION_RISK].value.c_str(),
                         SystemResults::results[SystemResults::IDX_DETECTION_RISK].color,
                         35 + SystemResults::IDX_DETECTION_RISK * 25);
}

void DisplayController::updateInterceptWindow(int hours, int minutes, int seconds) {
  char buf[16];
  sprintf(buf, "%02dh %02dm %02ds", hours, minutes, seconds);
  SystemResults::results[SystemResults::IDX_INTERCEPT_WINDOW].value = buf;
  renderSystemResultItem(SystemResults::results[SystemResults::IDX_INTERCEPT_WINDOW].label.c_str(),
                         SystemResults::results[SystemResults::IDX_INTERCEPT_WINDOW].value.c_str(),
                         SystemResults::results[SystemResults::IDX_INTERCEPT_WINDOW].color,
                         35 + SystemResults::IDX_INTERCEPT_WINDOW * 25);
}

void DisplayController::updateServiceLink(bool connected) {
  SystemResults::results[SystemResults::IDX_SERVICE_LINK].value =
      connected ? "CONNECTED" : "DISCONNECTED";
  renderSystemResultItem(SystemResults::results[SystemResults::IDX_SERVICE_LINK].label.c_str(),
                         SystemResults::results[SystemResults::IDX_SERVICE_LINK].value.c_str(),
                         SystemResults::results[SystemResults::IDX_SERVICE_LINK].color,
                         35 + SystemResults::IDX_SERVICE_LINK * 25);
}
