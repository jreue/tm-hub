#include "LEDStatusHelper.h"

LEDStatusHelper::LEDStatusHelper() {
  // Initialize calibration tracking
  for (int i = 0; i < 4; i++) {
    deviceCalibrated[i] = false;
  }
}

void LEDStatusHelper::begin() {
  FastLED.addLeds<WS2812, LED_DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(25);
  FastLED.clear(true);
  FastLED.show();
}

// Update LED display for a device
// Row 0: Available (Red = offline, Green = online)
// Row 1-11: Calibrated animation (when calibrated) or off (when not)
void LEDStatusHelper::updateStatusLEDs(int col, bool isAvailable, bool isCalibrated) {
  deviceCalibrated[col] = isCalibrated;

  if (!isAvailable) {
    // Device offline - red availability, everything else off
    leds[getLEDIndex(col, 0)] = CRGB::Red;
    for (int row = 1; row < MATRIX_ROWS; row++) {
      leds[getLEDIndex(col, row)] = CRGB::Black;
    }
  } else {
    // Device online - green availability
    leds[getLEDIndex(col, 0)] = CRGB::Green;

    if (!isCalibrated) {
      // Not calibrated - yellow on row 1, rest off
      leds[getLEDIndex(col, 1)] = CRGB::Orange;
      for (int row = 2; row < MATRIX_ROWS; row++) {
        leds[getLEDIndex(col, row)] = CRGB::Black;
      }
    }
    // If calibrated, animation will be handled by animate() method
  }
  FastLED.show();
}

// Animate calibrated devices with flowing energy effect
void LEDStatusHelper::animate() {
  unsigned long now = millis();
  float phase = (now % 2000) / 2000.0 * 2.0 * PI;  // 2 second cycle

  for (int col = 0; col < 4; col++) {
    if (deviceCalibrated[col]) {
      // Select color based on column
      CRGB baseColor;
      switch (col) {
        case 0:
          baseColor = CRGB(128, 0, 128);  // Purple
          break;
        case 1:
          baseColor = CRGB(0, 255, 0);    // Green
          break;
        case 2:
          baseColor = CRGB(0, 100, 255);  // Blue
          break;
        default:
          baseColor = CRGB(255, 255, 0);  // Yellow (for any additional columns)
          break;
      }

      // Animate rows 1-11 with flowing energy
      for (int row = 1; row < MATRIX_ROWS; row++) {
        // Create a wave that flows up the column
        float rowPhase = phase - (row - 1) * 0.3;        // Negative offset for upward flow
        float brightness = (sin(rowPhase) + 1.0) / 2.0;  // 0.0 to 1.0

        // Minimum brightness so LEDs don't fully turn off
        brightness = 0.2 + (brightness * 0.8);  // Range: 0.2 to 1.0

        // Apply brightness to the base color
        leds[getLEDIndex(col, row)] = CRGB(
          (uint8_t)(baseColor.r * brightness),
          (uint8_t)(baseColor.g * brightness),
          (uint8_t)(baseColor.b * brightness)
        );
      }
    }
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
