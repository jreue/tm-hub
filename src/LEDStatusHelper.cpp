#include "LEDStatusHelper.h"

LEDStatusHelper::LEDStatusHelper() {
  // Constructor - do nothing here
}

void LEDStatusHelper::begin() {
  FastLED.addLeds<WS2812, LED_DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(25);
  FastLED.clear(true);
  FastLED.show();
}

// Update LED display for a device
// Column 0: Available (Red = offline, Green = online)
// Column 1: Calibrated (Green = TRUE, Orange = FALSE)
void LEDStatusHelper::updateStatusLEDs(int row, bool isAvailable, bool isCalibrated) {
  if (!isAvailable) {
    leds[getLEDIndex(0, row)] = CRGB::Red;    // Availability: Red
    leds[getLEDIndex(1, row)] = CRGB::Black;  // Status: Off (no data)
  } else {
    leds[getLEDIndex(0, row)] = CRGB::Green;                                // Availability: Green
    leds[getLEDIndex(1, row)] = isCalibrated ? CRGB::Green : CRGB::Orange;  // Status
  }
  FastLED.show();
}

// Helper function to get LED index from column and row
// Handles serpentine/zigzag wiring pattern
int LEDStatusHelper::getLEDIndex(int col, int row) {
  if (row % 2 == 0) {
    // Even rows: left to right
    return row * MATRIX_COLS + col;
  } else {
    // Odd rows: right to left (serpentine)
    return row * MATRIX_COLS + (MATRIX_COLS - 1 - col);
  }
}
