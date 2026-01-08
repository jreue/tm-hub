#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>

class DisplayController {
  public:
    DisplayController();
    void begin(int deviceCount);
    void updateDeviceConnection(int count);
    void updateDeviceCalibration(int count);
    void updateScannerStatus(bool connected);
    void updateDate(uint8_t month, uint8_t day, uint16_t year);

  private:
    TFT_eSPI tft;
    int totalDevices;
    void enableBacklight();
};
