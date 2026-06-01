#include "LEDStatusHelper.h"

// ============================================================================
// Ring LED index mapping
//
// Each 16-LED ring is wired clockwise, with LED 0 and LED 15 both sitting at
// the 12 o'clock position (strip start and end meet at the top).
//
// Even device index -> cardinal positions (12 / 3 / 6 / 9 o'clock pairs)
// Odd  device index -> inter-cardinal positions (NE / SE / SW / NW pairs)
// ============================================================================

static const uint8_t CARDINAL_OFFSETS[8] = {0, 15, 3, 4, 7, 8, 11, 12};
static const uint8_t INTERCARDINAL_OFFSETS[8] = {1, 2, 5, 6, 9, 10, 13, 14};

// ============================================================================
// Breathing effect tuning
// Controls the minimum brightness at the bottom of each breath cycle.
// Range: 0.0 (fully off) to 1.0 (no breathing, always full brightness).
// Lower values = more dramatic pulse; higher values = subtle glow.
// ============================================================================
static constexpr float BREATH_MIN_OFFLINE = 0.05f;     // RED   — dips near-off
static constexpr float BREATH_MIN_ONLINE = 0.05f;      // GREEN — dips near-off
static constexpr float BREATH_MIN_CALIBRATED = 0.10f;  // PURPLE — slightly higher floor

// ============================================================================
// Constructor
// ============================================================================

LEDStatusHelper::LEDStatusHelper() {
  for (int i = 0; i < NUM_DEVICES; i++) {
    deviceAvailable[i] = false;
    deviceCalibrated[i] = false;
  }
  _activeEffect = TransientEffect::NONE;
  _effectStart = 0;
  _effectDeviceIndex = 0;
}

// ============================================================================
// Public Methods
// ============================================================================

void LEDStatusHelper::begin() {
  FastLED.addLeds<WS2812, LED_DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(125);  // 0-255 brightness scale
  FastLED.clear(true);
  FastLED.show();
}

bool LEDStatusHelper::isEffectActive() const {
  return _activeEffect != TransientEffect::NONE;
}

// Records the connection/calibration state of a device.
// Detects state transitions and triggers transient event effects.
// animate() owns all rendering — no FastLED.show() here.
void LEDStatusHelper::updateStatusLEDs(int deviceIndex, bool isAvailable, bool isCalibrated) {
  if (deviceIndex < 0 || deviceIndex >= NUM_DEVICES)
    return;

  bool wasAvailable = deviceAvailable[deviceIndex];
  bool wasCalibrated = deviceCalibrated[deviceIndex];

  // Trigger connected effect when a device comes online
  if (!wasAvailable && isAvailable) {
    _activeEffect = TransientEffect::CONNECTED;
    _effectStart = millis();
    _effectDeviceIndex = deviceIndex;
  }
  // Trigger celebration when a device becomes calibrated
  else if (wasAvailable && !wasCalibrated && isCalibrated) {
    _activeEffect = TransientEffect::CALIBRATION_CELEBRATION;
    _effectStart = millis();
    _effectDeviceIndex = deviceIndex;
  }

  deviceAvailable[deviceIndex] = isAvailable;
  deviceCalibrated[deviceIndex] = isCalibrated;
}

void LEDStatusHelper::triggerTravelEffect() {
  _activeEffect = TransientEffect::TRAVEL;
  _effectStart = millis();
}

// Called every loop tick. Dispatches transient event effects when active,
// otherwise renders per-device status. Owns the single FastLED.show() call.
void LEDStatusHelper::animate() {
  if (_activeEffect != TransientEffect::NONE) {
    bool done = false;
    if (_activeEffect == TransientEffect::CONNECTED) {
      done = renderConnectedEffect(_effectDeviceIndex);
    } else if (_activeEffect == TransientEffect::CALIBRATION_CELEBRATION) {
      done = renderCalibrationCelebrationEffect(_effectDeviceIndex);
    } else if (_activeEffect == TransientEffect::TRAVEL) {
      done = renderTravelEffect();
    }
    if (done)
      _activeEffect = TransientEffect::NONE;
  } else {
    for (int i = 0; i < NUM_DEVICES; i++) {
      if (!deviceAvailable[i]) {
        renderOfflineStateEffect(i);
      } else if (!deviceCalibrated[i]) {
        renderOnlineStateEffect(i);
      } else {
        renderCalibrationStateEffect(i);
      }
    }
  }
  FastLED.show();
}

// ============================================================================
// Private — LED Index Helper
// ============================================================================

// Fills indices[8] with the global LED array positions for the given device.
// Even device index -> cardinal positions (12/3/6/9 o'clock pairs).
// Odd  device index -> inter-cardinal positions (NE/SE/SW/NW pairs).
void LEDStatusHelper::getDeviceLEDIndices(int deviceIndex, uint8_t indices[8]) {
  uint8_t ringBase = (deviceIndex / 2) * LEDS_PER_RING;
  const uint8_t* offsets = (deviceIndex % 2 == 0) ? CARDINAL_OFFSETS : INTERCARDINAL_OFFSETS;
  for (int i = 0; i < 8; i++) {
    indices[i] = ringBase + offsets[i];
  }
}

// ============================================================================
// Private — Named Effect Functions
// ============================================================================

// Offline: RED gentle breath — 4-second cycle, brightness 0.3 to 1.0
void LEDStatusHelper::renderOfflineStateEffect(int deviceIndex) {
  uint8_t indices[8];
  getDeviceLEDIndices(deviceIndex, indices);

  unsigned long now = millis();
  unsigned long offset = (deviceIndex * 4000UL) / NUM_DEVICES;
  float phase = ((now + offset) % 4000UL) / 4000.0f * 2.0f * PI;
  float brightness =
      BREATH_MIN_OFFLINE + ((sinf(phase) + 1.0f) / 2.0f) * (1.0f - BREATH_MIN_OFFLINE);

  for (int i = 0; i < 8; i++) {
    leds[indices[i]] = CRGB((uint8_t)(255 * brightness), 0, 0);
  }
}

// Online (connected, not calibrated): GREEN gentle breath — 3-second cycle, brightness 0.3 to 1.0
void LEDStatusHelper::renderOnlineStateEffect(int deviceIndex) {
  uint8_t indices[8];
  getDeviceLEDIndices(deviceIndex, indices);

  unsigned long now = millis();
  unsigned long offset = (deviceIndex * 3000UL) / NUM_DEVICES;
  float phase = ((now + offset) % 3000UL) / 3000.0f * 2.0f * PI;
  float brightness = BREATH_MIN_ONLINE + ((sinf(phase) + 1.0f) / 2.0f) * (1.0f - BREATH_MIN_ONLINE);

  for (int i = 0; i < 8; i++) {
    leds[indices[i]] = CRGB(0, (uint8_t)(255 * brightness), 0);
  }
}

// Calibrated: PURPLE gentle breath — 2-second cycle, brightness 0.4 to 1.0
void LEDStatusHelper::renderCalibrationStateEffect(int deviceIndex) {
  uint8_t indices[8];
  getDeviceLEDIndices(deviceIndex, indices);

  unsigned long now = millis();
  unsigned long offset = (deviceIndex * 2000UL) / NUM_DEVICES;
  float phase = ((now + offset) % 2000UL) / 2000.0f * 2.0f * PI;
  float brightness =
      BREATH_MIN_CALIBRATED + ((sinf(phase) + 1.0f) / 2.0f) * (1.0f - BREATH_MIN_CALIBRATED);

  for (int i = 0; i < 8; i++) {
    leds[indices[i]] = CRGB((uint8_t)(128 * brightness), 0, (uint8_t)(128 * brightness));
  }
}

// ============================================================================
// Private — Shared Comet Helper
// ============================================================================

// Renders 3 evenly-spaced comets racing clockwise around one 16-LED ring.
// ringBase  : first global LED index of the ring in the strip.
// headPos   : current head position in LED units (fractional; wraps at LEDS_PER_RING).
// color     : base CRGB color of the comet head.
// brightness: overall scale 0.0–1.0 used to fade the ring in/out between phases.
void LEDStatusHelper::renderRingComets(int ringBase, float headPos, CRGB color, float brightness,
                                       bool clockwise) {
  const int NUM_COMETS = 3;
  const int TAIL_LENGTH = 5;
  const float TAIL_DECAY = 0.55f;  // brightness multiplier per tail LED

  for (int c = 0; c < NUM_COMETS; c++) {
    float pos = fmodf(headPos + c * (LEDS_PER_RING / (float)NUM_COMETS), LEDS_PER_RING);
    int headIdx = (int)pos % LEDS_PER_RING;

    // Head — full brightness
    leds[ringBase + headIdx] +=
        CRGB((uint8_t)(color.r * brightness), (uint8_t)(color.g * brightness),
             (uint8_t)(color.b * brightness));

    // Tail — exponential fade, direction follows comet motion
    float tailBr = brightness;
    for (int t = 1; t <= TAIL_LENGTH; t++) {
      tailBr *= TAIL_DECAY;
      int tailIdx =
          clockwise ? (headIdx - t + LEDS_PER_RING) % LEDS_PER_RING : (headIdx + t) % LEDS_PER_RING;
      leds[ringBase + tailIdx] += CRGB((uint8_t)(color.r * tailBr), (uint8_t)(color.g * tailBr),
                                       (uint8_t)(color.b * tailBr));
    }
  }
}

// ============================================================================
// Private — Transient Event Effects
// ============================================================================

// Electrical energy spirals into the device's ring, expands to all 3 rings,
// then retracts. Color transitions from blue to green. Returns true when done.
// Duration: 5 seconds.
bool LEDStatusHelper::renderConnectedEffect(int deviceIndex) {
  const unsigned long DURATION = 5000;
  const unsigned long PHASE2_MS = 1500;  // expand to all rings
  const unsigned long PHASE3_MS = 3500;  // retract back to device ring

  unsigned long elapsed = millis() - _effectStart;
  if (elapsed >= DURATION)
    return true;

  float t = elapsed / (float)DURATION;                  // 0.0 → 1.0
  float headPos = (elapsed / 1000.0f) * LEDS_PER_RING;  // 1 revolution per second

  // Color lerp: blue (0,0,255) → green (0,255,0)
  CRGB color = CRGB(0, (uint8_t)(255 * t), (uint8_t)(255 * (1.0f - t)));

  int deviceRingBase = (deviceIndex / 2) * LEDS_PER_RING;

  fill_solid(leds, NUM_LEDS, CRGB::Black);

  if (elapsed < PHASE2_MS) {
    // Phase 1: comets on device ring only
    renderRingComets(deviceRingBase, headPos, color, 1.0f);

  } else if (elapsed < PHASE3_MS) {
    // Phase 2: comets expand to all rings (other rings fade in over 400ms)
    float otherFade = (elapsed - PHASE2_MS) / 400.0f;
    if (otherFade > 1.0f)
      otherFade = 1.0f;
    renderRingComets(deviceRingBase, headPos, color, 1.0f);
    for (int ring = 0; ring < NUM_RINGS; ring++) {
      int base = ring * LEDS_PER_RING;
      if (base != deviceRingBase) {
        renderRingComets(base, headPos, color, otherFade);
      }
    }

  } else {
    // Phase 3: other rings fade out, comets retract to device ring
    float otherFade = 1.0f - (elapsed - PHASE3_MS) / (float)(DURATION - PHASE3_MS);
    if (otherFade < 0.0f)
      otherFade = 0.0f;
    renderRingComets(deviceRingBase, headPos, color, 1.0f);
    for (int ring = 0; ring < NUM_RINGS; ring++) {
      int base = ring * LEDS_PER_RING;
      if (base != deviceRingBase) {
        renderRingComets(base, headPos, color, otherFade);
      }
    }
  }

  return false;
}

// Celebratory purple burst: comets accelerate across all rings, erupt into sparkles,
// then gracefully settle back into the calibration state glow. Returns true when done.
// Duration: 8 seconds.
bool LEDStatusHelper::renderCalibrationCelebrationEffect(int deviceIndex) {
  const unsigned long DURATION = 8000;
  const unsigned long PHASE2_MS = 3000;  // sparkle burst begins
  const unsigned long PHASE3_MS = 6000;  // decelerate and dissolve

  unsigned long elapsed = millis() - _effectStart;
  if (elapsed >= DURATION)
    return true;

  fill_solid(leds, NUM_LEDS, CRGB::Black);

  if (elapsed < PHASE2_MS) {
    // Phase 1: comets accelerate across all rings (0.5 → 3 rev/sec)
    // Integrated position: pos = 0.5t + (2.5/6)t²  (revolutions, t in seconds)
    float sec = elapsed / 1000.0f;
    float pos = (0.5f * sec + (2.5f / 6.0f) * sec * sec) * LEDS_PER_RING;
    CRGB purple = CRGB(128, 0, 128);
    for (int ring = 0; ring < NUM_RINGS; ring++) {
      renderRingComets(ring * LEDS_PER_RING, pos, purple, 1.0f);
    }

  } else if (elapsed < PHASE3_MS) {
    // Phase 2: deterministic sparkle burst over all 48 LEDs + fast comets
    // Position continues from end of phase 1 (5.25 revolutions) at 3 rev/sec
    float sec = elapsed / 1000.0f;
    float pos = (5.25f + 3.0f * (sec - 3.0f)) * LEDS_PER_RING;

    // Sparkle: two interleaved sin waves with golden-ratio phase spacing
    // creates organic, pseudo-random flickering without needing state
    for (int i = 0; i < NUM_LEDS; i++) {
      float sp = sinf(sec * 8.0f + i * 2.3999f) * sinf(sec * 5.0f + i * 1.6180f);
      float br = (sp + 1.0f) / 2.0f;  // 0.0 → 1.0
      // Shift hue between purple (128,0,128), violet (148,0,211), magenta (200,0,128)
      float hue = sinf(sec * 3.0f + i * 0.5f);  // -1.0 → 1.0
      uint8_t r = (uint8_t)(((hue + 1.0f) / 2.0f * 80.0f + 100.0f) * br);
      uint8_t b = (uint8_t)(((-hue + 1.0f) / 2.0f * 83.0f + 128.0f) * br);
      leds[i] = CRGB(r, 0, b);
    }

    // Fast comets on top (additive blend)
    CRGB cometColor = CRGB(180, 50, 200);
    for (int ring = 0; ring < NUM_RINGS; ring++) {
      renderRingComets(ring * LEDS_PER_RING, pos, cometColor, 0.8f);
    }

  } else {
    // Phase 3: comets decelerate (3 → 0 rev/sec) and fade; calibration glow fades in
    // Position continues from end of phase 2 (14.25 revolutions) with deceleration
    // Integrated: pos = 14.25 + 3u - 0.75u²  (u = seconds into phase 3)
    float u = (elapsed - PHASE3_MS) / 1000.0f;
    float pos = (14.25f + 3.0f * u - 0.75f * u * u) * LEDS_PER_RING;
    float phase3t = (elapsed - PHASE3_MS) / (float)(DURATION - PHASE3_MS);  // 0 → 1
    float cometBr = 1.0f - phase3t;

    CRGB purple = CRGB(128, 0, 128);
    for (int ring = 0; ring < NUM_RINGS; ring++) {
      renderRingComets(ring * LEDS_PER_RING, pos, purple, cometBr);
    }

    // Fade in calibration state glow for this device
    uint8_t indices[8];
    getDeviceLEDIndices(deviceIndex, indices);
    for (int i = 0; i < 8; i++) {
      leds[indices[i]] += CRGB((uint8_t)(128 * phase3t), 0, (uint8_t)(128 * phase3t));
    }
  }

  return false;
}

// ============================================================================
// Time Travel Effect — 18 seconds, audio-synced to the travel sequence
// Phase 1 (0–4s):   Electrical charge — blue-white sparks from darkness to full
// Phase 2 (4–13s):  Time Storm — counter-rotating rainbow comets + sparkle chaos
// Phase 3 (13–16s): Temporal Vortex — synced rings, depth flash, 10Hz pulse
// Phase 4 (16–18s): Landing — decelerate, white flash, dissolve to status
// ============================================================================
bool LEDStatusHelper::renderTravelEffect() {
  const unsigned long DURATION = 18000;
  const unsigned long PHASE2_MS = 4000;
  const unsigned long PHASE3_MS = 13000;
  const unsigned long PHASE4_MS = 16000;

  unsigned long elapsed = millis() - _effectStart;
  if (elapsed >= DURATION)
    return true;

  fill_solid(leds, NUM_LEDS, CRGB::Black);

  if (elapsed < PHASE2_MS) {
    // -----------------------------------------------------------------------
    // Phase 1: Electrical Charge Up (0–4s)
    // Blue-white sparks grow from sparse to dense as energy builds
    // -----------------------------------------------------------------------
    float t = elapsed / (float)PHASE2_MS;  // 0.0 → 1.0
    float sec = elapsed / 1000.0f;

    for (int i = 0; i < NUM_LEDS; i++) {
      // Each LED has a unique fast-flickering frequency (golden-ratio spacing)
      float sp = sinf(sec * (8.0f + fmodf(i * 4.17f, 14.0f)) + i * 1.6180f);
      sp = (sp + 1.0f) / 2.0f;              // 0.0 → 1.0
      float threshold = 0.92f - t * 0.82f;  // 0.92 → 0.10
      float br = sp > threshold ? (sp - threshold) / (1.0f - threshold) : 0.0f;
      br *= t;  // global brightness ramp from black

      // Secondary oscillation adds occasional white flashes
      float flash = sinf(sec * 29.0f + i * 3.7321f);
      flash = (flash + 1.0f) / 2.0f;
      float whiteMix = flash * t * 0.4f;

      leds[i] = CRGB((uint8_t)((80.0f + 175.0f * whiteMix) * br),
                     (uint8_t)((120.0f + 135.0f * whiteMix) * br), (uint8_t)(255.0f * br));
    }

  } else if (elapsed < PHASE3_MS) {
    // -----------------------------------------------------------------------
    // Phase 2: Time Storm (4–13s)
    // Counter-rotating rainbow comets over dense hue-cycling sparkle chaos
    // -----------------------------------------------------------------------
    float sec = elapsed / 1000.0f;
    float headPos = (sec - 4.0f) * 3.5f * LEDS_PER_RING;     // 3.5 rev/sec since phase start
    uint8_t baseHue = (uint8_t)((elapsed - PHASE2_MS) / 8);  // full rainbow cycle every ~2s

    // Sparkle base layer: rapid rainbow flicker across all 48 LEDs
    for (int i = 0; i < NUM_LEDS; i++) {
      float sp = sinf(sec * 12.0f + i * 2.3999f) * sinf(sec * 7.0f + i * 1.6180f);
      float br = (sp + 1.0f) / 2.0f;
      CRGB col;
      hsv2rgb_rainbow(CHSV(baseHue + (uint8_t)(i * 5), 255, (uint8_t)(200 * br)), col);
      leds[i] = col;
    }

    // Counter-rotating comets — center/right CW, left ring CCW
    for (int ring = 0; ring < NUM_RINGS; ring++) {
      bool cw = (ring != 1);  // ring 1 = left ring, spins CCW
      float pos =
          cw ? headPos : fmodf(LEDS_PER_RING - fmodf(headPos, LEDS_PER_RING), LEDS_PER_RING);
      uint8_t ringHue = baseHue + (uint8_t)(ring * 85);
      CRGB cometColor;
      hsv2rgb_rainbow(CHSV(ringHue, 255, 255), cometColor);
      renderRingComets(ring * LEDS_PER_RING, pos, cometColor, 1.0f, cw);
    }

  } else if (elapsed < PHASE4_MS) {
    // -----------------------------------------------------------------------
    // Phase 3: Temporal Vortex (13–16s)
    // All rings locked together, fast ice-blue spin, depth-tunnel flash, 10Hz throb
    // -----------------------------------------------------------------------
    float sec = elapsed / 1000.0f;
    // Continue position from end of Phase 2 (31.5 rev) at 5 rev/sec
    float pos = (31.5f + 5.0f * (sec - 13.0f)) * LEDS_PER_RING;

    // Sequential ring depth flash: center → left → right, 200ms each, 600ms cycle
    int flashRing = (int)(((elapsed - PHASE3_MS) % 600UL) / 200);

    // 10Hz throb pulse
    float pulseSec = (elapsed - PHASE3_MS) / 1000.0f;
    float pulse = 0.55f + 0.45f * sinf(pulseSec * 10.0f * 2.0f * PI);

    CRGB iceBlue = CRGB(100, 200, 255);
    for (int ring = 0; ring < NUM_RINGS; ring++) {
      float ringBr = (ring == flashRing ? 1.0f : 0.65f) * pulse;
      renderRingComets(ring * LEDS_PER_RING, pos, iceBlue, ringBr);
    }

  } else {
    // -----------------------------------------------------------------------
    // Phase 4: Landing (16–18s)
    // Comets decelerate; white flash peaks at t=17s; status glow fades back in
    // -----------------------------------------------------------------------
    float u = (elapsed - PHASE4_MS) / 1000.0f;  // 0 → 2 seconds
    float phase4t = u / 2.0f;                   // 0.0 → 1.0
    // Integrated decel: speed 5→0 rev/sec over 2s → pos = 46.5 + 5u − 1.25u²
    float pos = (46.5f + 5.0f * u - 1.25f * u * u) * LEDS_PER_RING;

    CRGB iceBlue = CRGB(100, 200, 255);
    float cometBr = 1.0f - phase4t;
    for (int ring = 0; ring < NUM_RINGS; ring++) {
      renderRingComets(ring * LEDS_PER_RING, pos, iceBlue, cometBr);
    }

    // White flash: narrow bell curve peaking at phase4t=0.5 (t=17s)
    float flashBr = 1.0f - fabsf(phase4t - 0.5f) * 6.0f;
    if (flashBr > 0.0f) {
      uint8_t flashVal = (uint8_t)(255 * flashBr);
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] += CRGB(flashVal, flashVal, flashVal);
      }
    }

    // Status glow fades in over the final third of the landing
    if (phase4t > 0.65f) {
      float statusFade = (phase4t - 0.65f) / 0.35f;
      for (int i = 0; i < NUM_DEVICES; i++) {
        uint8_t devIndices[8];
        getDeviceLEDIndices(i, devIndices);
        CRGB statusColor;
        if (!deviceAvailable[i])
          statusColor = CRGB((uint8_t)(128 * statusFade), 0, 0);
        else if (!deviceCalibrated[i])
          statusColor = CRGB(0, (uint8_t)(128 * statusFade), 0);
        else
          statusColor = CRGB((uint8_t)(64 * statusFade), 0, (uint8_t)(64 * statusFade));
        for (int j = 0; j < 8; j++) {
          leds[devIndices[j]] += statusColor;
        }
      }
    }
  }

  return false;
}
