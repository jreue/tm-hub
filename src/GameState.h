#pragma once

#include <Arduino.h>
#include <MessageStructs.h>
#include <Preferences.h>

namespace StorageKeys {
// Namespace key
const char* const STORAGE_NAMESPACE = "game-state";

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

// Intercept window keys
const char* const INTERCEPT_HOURS = "timer-hours";
const char* const INTERCEPT_MINUTES = "timer-minutes";
const char* const INTERCEPT_SECONDS = "timer-seconds";

// Shield module keys
const char* const MODULE_COUNT = "module-count";
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
const int INTERCEPT_HOURS = 48;
const int INTERCEPT_MINUTES = 0;
const int INTERCEPT_SECONDS = 0;
}  // namespace Defaults

class GameState {
  public:
    GameState();

    void begin();
    void load();
    void save();
    void reset();

    void setTargetDate(uint8_t month, uint8_t day, uint16_t year);
    void getTargetDate(uint8_t& month, uint8_t& day, uint16_t& year);
    void setCurrentDate(uint8_t month, uint8_t day, uint16_t year);
    void getCurrentDate(uint8_t& month, uint8_t& day, uint16_t& year);
    void setLastDate(uint8_t month, uint8_t day, uint16_t year);
    void getLastDate(uint8_t& month, uint8_t& day, uint16_t& year);

    // Intercept window
    void setInterceptWindowTime(int hours, int minutes, int seconds);
    void getInterceptWindowTime(int& hours, int& minutes, int& seconds);
    void tickCountdown();

    // Shield module states
    void setShieldModuleState(int index, bool available, bool calibrated);
    void getShieldModuleState(int index, bool& available, bool& calibrated);
    ModuleState* getShieldModuleStates();

    // Date rolling (for travel button)
    void rollDates();

    // Target date validation
    bool isValidTargetDate();

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
    int interceptHours;
    int interceptMinutes;
    int interceptSeconds;

    // Shield module states (max 10 shield modules)
    ModuleState shieldModuleStates[10];
    int numShieldModules;
};
