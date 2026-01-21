#pragma once

#include <Arduino.h>
#include <MessageStructs.h>
#include <Preferences.h>

namespace StorageKeys {
// Date keys
const char* const TARGET_MONTH = "target-month";
const char* const TARGET_DAY = "target-day";
const char* const TARGET_YEAR = "target-year";
const char* const CURRENT_MONTH = "current-month";
const char* const CURRENT_DAY = "current-day";
const char* const CURRENT_YEAR = "current-year";
const char* const LAST_MONTH = "last-month";
const char* const LAST_DAY = "last-day";
const char* const LAST_YEAR = "last-year";

// Time key
const char* const TIME_REMAINING = "time-remaining";

// Device keys
const char* const DEVICE_COUNT = "device-count";
}  // namespace StorageKeys

// Default values
namespace Defaults {
const uint8_t TARGET_MONTH = 0;
const uint8_t TARGET_DAY = 0;
const uint16_t TARGET_YEAR = 0;
const uint8_t CURRENT_MONTH = 1;
const uint8_t CURRENT_DAY = 1;
const uint16_t CURRENT_YEAR = 2056;
const uint8_t LAST_MONTH = 12;
const uint8_t LAST_DAY = 25;
const uint16_t LAST_YEAR = 2025;
const int INTERCEPT_WINDOW_SECONDS = 48 * 60 * 60;
}  // namespace Defaults

class GameState {
  public:
    GameState();

    void begin();
    void load();
    void save();

    void setTargetDate(uint8_t month, uint8_t day, uint16_t year);
    void getTargetDate(uint8_t& month, uint8_t& day, uint16_t& year);
    void setCurrentDate(uint8_t month, uint8_t day, uint16_t year);
    void getCurrentDate(uint8_t& month, uint8_t& day, uint16_t& year);
    void setLastDate(uint8_t month, uint8_t day, uint16_t year);
    void getLastDate(uint8_t& month, uint8_t& day, uint16_t& year);

    // Intercept window
    void setInterceptWindowSeconds(int seconds);
    int getInterceptWindowSeconds();
    void getInterceptWindowTime(int& hours, int& minutes, int& seconds);

    // Device states
    void setDeviceState(int index, bool available, bool calibrated);
    void getDeviceState(int index, bool& available, bool& calibrated);
    DeviceState* getDeviceStates();

    // Date rolling (for travel button)
    void rollDates();

  private:
    Preferences preferences;

    // Date state tracking
    uint8_t targetMonth, targetDay;
    uint16_t targetYear;
    uint8_t currentMonth, currentDay;
    uint16_t currentYear;
    uint8_t lastMonth, lastDay;
    uint16_t lastYear;

    // Intercept window
    int interceptWindowSeconds;
    int interceptHours;
    int interceptMinutes;
    int interceptSeconds;

    // Device states (max 10 devices)
    DeviceState deviceStates[10];
    int numDevices;

    void updateInterceptTimeComponents();
};
