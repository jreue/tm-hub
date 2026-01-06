#pragma once

#include <Arduino.h>
#include <FastLED.h>

#include "hardware_config.h"

class LEDStatusHelper {
  public:
    LEDStatusHelper();
    void begin();
    void updateStatusLEDs(int row, bool isAvailable, bool isCalibrated);

  private:
    CRGB leds[NUM_LEDS];
    int getLEDIndex(int col, int row);
};
