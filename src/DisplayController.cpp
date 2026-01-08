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
  tft.drawString("TM Hub", 10, 10);
}

void DisplayController::enableBacklight() {
  digitalWrite(TFT_BL, HIGH);
}

void DisplayController::updateDeviceConnection(int count) {
  tft.fillRect(0, 30, tft.width(), 20, TFT_BLACK);
  tft.drawString("Shield Modules Online: " + String(count) + " / " + String(totalDevices), 10, 30);
}

void DisplayController::updateDeviceCalibration(int count) {
  tft.fillRect(0, 50, tft.width(), 20, TFT_BLACK);
  tft.drawString("Shield Modules Calibrated: " + String(count) + " / " + String(totalDevices), 10,
                 50);
}

void DisplayController::updateScannerStatus(bool connected) {
  tft.fillRect(0, 200, tft.width(), 20, TFT_BLACK);
  if (connected) {
    tft.drawString("Scanner: CONNECTED", 10, 200);
  } else {
    tft.drawString("Scanner: DISCONNECTED", 10, 200);
  }
}

void DisplayController::updateDate(uint8_t month, uint8_t day, uint16_t year) {
  // Update TFT (at bottom of 320x480 portrait display)
  tft.setTextSize(2);
  tft.fillRect(0, 440, tft.width(), 40, TFT_BLACK);
  tft.drawString("Date: " + String(month) + "/" + String(day) + "/" + String(year), 10, 450);
  tft.setTextSize(1);  // Reset to default size
}
