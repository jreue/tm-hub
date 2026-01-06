#pragma once

#include <Arduino.h>
#include <FastLED.h>

#include "hardware_config.h"

class LEDStatusHelper {
  public:
    LEDStatusHelper();
    void begin();
    void updateStatusLEDs(int col, bool isAvailable, bool isCalibrated);
    void animate();

  private:
    CRGB leds[NUM_LEDS];
    bool deviceCalibrated[4];          // Track which devices are calibrated
    unsigned long calibrationTime[4];  // When each device became calibrated
    int getLEDIndex(int col, int row);
};
