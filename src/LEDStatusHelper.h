#pragma once

#include <Arduino.h>
#include <FastLED.h>

#include "hardware_config.h"

class LEDStatusHelper {
  public:
    LEDStatusHelper();
    void begin();
    void updateStatusLEDs(int deviceIndex, bool isAvailable, bool isCalibrated);
    void triggerTravelEffect();
    void animate();

  private:
    CRGB leds[NUM_LEDS];
    bool deviceAvailable[NUM_DEVICES];   // Track which devices are connected
    bool deviceCalibrated[NUM_DEVICES];  // Track which devices are calibrated

    // Transient event effect state
    enum class TransientEffect { NONE, CONNECTED, CALIBRATION_CELEBRATION, TRAVEL };
    TransientEffect _activeEffect;
    unsigned long _effectStart;
    int _effectDeviceIndex;

    // Maps a device index (0-5) to its 8 physical LED indices in the strip
    void getDeviceLEDIndices(int deviceIndex, uint8_t indices[8]);

    // Shared comet rendering helper
    void renderRingComets(int ringBase, float headPos, CRGB color, float brightness,
                          bool clockwise = true);

    // Status state effects — called by animate() when no transient effect is active
    void renderOfflineStateEffect(int deviceIndex);      // RED gentle breath
    void renderOnlineStateEffect(int deviceIndex);       // GREEN gentle breath
    void renderCalibrationStateEffect(int deviceIndex);  // PURPLE gentle breath

    // Transient event effects — return true when the effect has finished
    bool renderConnectedEffect(int deviceIndex);
    bool renderCalibrationCelebrationEffect(int deviceIndex);
    bool renderTravelEffect();
};
