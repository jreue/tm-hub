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

  ///////////

  const char* CorePowerValue_text = "ONLINE";
  const char* CurrentDateValue_text = "06/04/2056";
  const char* DestinationValue_text = "--/--/--";
  const char* LastDepartureValue_text = "06/04/2026";
  const char* PowerValue_text = "33%";
  const char* RiskValue_text = "HIGH";
  const char* ServiceLinkValue_text = "DISCONNECTED";

  // Header Text
  tft.setTextColor(0x3FE3);
  tft.setTextSize(1);
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.drawString("TIME DISPLACEMENT CONSOLE", 23, 7);
  // Header Underline
  tft.drawLine(10, 30, 310, 30, 0x3FE3);
  // Core Power Label
  tft.drawString("Core Power", 15, 421);
  // Service Link Label
  tft.drawString("Service Link", 15, 449);
  // Bottom Line
  tft.drawLine(10, 410, 310, 410, 0x3FE3);
  // Intercept Window Label
  tft.drawString("INTERCEPT WINDOW", 72, 336);
  // Intercept Time
  tft.setTextColor(0xFD80);
  tft.setFreeFont(&FreeMonoBold24pt7b);
  tft.drawString("48:53:33", 48, 356);
  // Destination Label
  tft.setTextColor(0x3FE2);
  tft.setFreeFont(&FreeMonoBold9pt7b);
  tft.drawString("Destination Date", 15, 44);
  // Current Label
  tft.drawString("Current Date", 15, 72);
  // Last Departure Label
  tft.drawString("Last Departure", 15, 100);
  // Shield Border
  tft.drawRoundRect(10, 149, 300, 174, 5, 0x3FE2);
  // Shield Container
  tft.fillRoundRect(86, 140, 149, 18, 6, 0x3FE2);
  // Shielding Header
  tft.setTextColor(0x0);
  tft.drawString("SHIELDING", 111, 141);
  // Modules Label
  tft.setTextColor(0x3FE2);
  tft.drawString("Modules", 25, 184);
  // Calibrated Label
  tft.drawString("Calibrated", 25, 218);
  // Power Label
  tft.drawString("Power", 25, 252);
  // Risk Label
  tft.drawString("Detection Risk", 25, 285);
  // DestinationValue
  tft.drawString(DestinationValue_text, 219, 44);
  // CurrentDateValue
  tft.drawString(CurrentDateValue_text, 197, 72);
  // LastDepartureValue
  tft.drawString(LastDepartureValue_text, 196, 100);
  // PowerValue
  tft.drawString(PowerValue_text, 253, 252);
  // RiskValue
  tft.setTextColor(0xF9C6);
  tft.drawString(RiskValue_text, 245, 285);
  // ServiceLinkValue
  tft.drawString(ServiceLinkValue_text, 174, 449);
  // CorePowerValue
  tft.setTextColor(0xFFFF);
  tft.drawString(CorePowerValue_text, 240, 421);
  // ModuleValue1
  tft.fillRect(180, 181, 10, 20, 0x3FEC);
  // ModuleValue2
  tft.fillRect(194, 181, 10, 20, 0x19E3);
  // ModuleValue3
  tft.fillRect(208, 181, 10, 20, 0x19E3);
  // ModuleValue4
  tft.fillRect(222, 181, 10, 20, 0x19E3);
  // ModuleValue5
  tft.fillRect(236, 181, 10, 20, 0x19E3);
  // ModuleValue6
  tft.fillRect(250, 181, 10, 20, 0x19E3);
  // ModuleValue7
  tft.fillRect(264, 181, 10, 20, 0x19E3);
  // ModuleValue8
  tft.fillRect(278, 181, 10, 20, 0x19E3);
  // Calibrated1Value
  tft.fillRect(180, 214, 10, 20, 0x3FEC);
  // Calibrated2Value
  tft.fillRect(194, 214, 10, 20, 0x19E3);
  // Calibrated3Value
  tft.fillRect(208, 214, 10, 20, 0x19E3);
  // Calibrated4Value
  tft.fillRect(222, 214, 10, 20, 0x19E3);
  // Calibrated5Value
  tft.fillRect(236, 214, 10, 20, 0x19E3);
  // Calibrated6Value
  tft.fillRect(250, 214, 10, 20, 0x19E3);
  // Calibrated7Value
  tft.fillRect(264, 214, 10, 20, 0x19E3);
  // Calibrated8Value
  tft.fillRect(278, 214, 10, 20, 0x19E3);
}

void DisplayController::enableBacklight() {
  digitalWrite(TFT_BL, HIGH);
}

void DisplayController::updateDestinationDate(uint8_t month, uint8_t day, uint16_t year) {
  // Stub - implementation removed
}

void DisplayController::updateCurrentDate(uint8_t month, uint8_t day, uint16_t year) {
  // Stub - implementation removed
}

void DisplayController::updateLastDeparture(uint8_t month, uint8_t day, uint16_t year) {
  // Stub - implementation removed
}

void DisplayController::updateCorePower(const String& status) {
  // Stub - implementation removed
}

void DisplayController::updateShieldModules(int online, int calibrated) {
  // Stub - implementation removed
}

void DisplayController::updateShieldStatus(const String& status) {
  // Stub - implementation removed
}

void DisplayController::updateDetectionRisk(const String& level) {
  // Stub - implementation removed
}

void DisplayController::updateInterceptWindow(int hours, int minutes, int seconds) {
  // Stub - implementation removed
}

void DisplayController::updateServiceLink(bool connected) {
  // Stub - implementation removed
}
