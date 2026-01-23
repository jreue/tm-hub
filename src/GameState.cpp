#include "GameState.h"

GameState::GameState()
    : targetMonth(Defaults::TARGET_MONTH),
      targetDay(Defaults::TARGET_DAY),
      targetYear(Defaults::TARGET_YEAR),
      currentMonth(Defaults::CURRENT_MONTH),
      currentDay(Defaults::CURRENT_DAY),
      currentYear(Defaults::CURRENT_YEAR),
      lastMonth(Defaults::LAST_MONTH),
      lastDay(Defaults::LAST_DAY),
      lastYear(Defaults::LAST_YEAR),
      interceptWindowSeconds(Defaults::INTERCEPT_WINDOW_SECONDS),
      interceptHours(48),
      interceptMinutes(0),
      interceptSeconds(0),
      numShieldModules(0) {
  // Initialize shield module states
  for (int i = 0; i < 10; i++) {
    shieldModuleStates[i].available = false;
    shieldModuleStates[i].calibrated = false;
  }
}

void GameState::begin() {
  // Nothing needed here for now
}

void GameState::load() {
  preferences.begin(StorageKeys::STORAGE_NAMESPACE, true);  // Open in read-only mode

  // Load target date
  targetMonth = preferences.getUChar(StorageKeys::TARGET_MONTH, Defaults::TARGET_MONTH);
  targetDay = preferences.getUChar(StorageKeys::TARGET_DAY, Defaults::TARGET_DAY);
  targetYear = preferences.getUShort(StorageKeys::TARGET_YEAR, Defaults::TARGET_YEAR);

  // Load current date
  currentMonth = preferences.getUChar(StorageKeys::CURRENT_MONTH, Defaults::CURRENT_MONTH);
  currentDay = preferences.getUChar(StorageKeys::CURRENT_DAY, Defaults::CURRENT_DAY);
  currentYear = preferences.getUShort(StorageKeys::CURRENT_YEAR, Defaults::CURRENT_YEAR);

  // Load last date
  lastMonth = preferences.getUChar(StorageKeys::LAST_MONTH, Defaults::LAST_MONTH);
  lastDay = preferences.getUChar(StorageKeys::LAST_DAY, Defaults::LAST_DAY);
  lastYear = preferences.getUShort(StorageKeys::LAST_YEAR, Defaults::LAST_YEAR);

  // Load intercept window
  interceptWindowSeconds =
      preferences.getInt(StorageKeys::TIME_REMAINING, Defaults::INTERCEPT_WINDOW_SECONDS);
  updateInterceptTimeComponents();

  // Load number of shield modules
  numShieldModules = preferences.getInt(StorageKeys::MODULE_COUNT, 0);

  // Load shield module states
  for (int i = 0; i < numShieldModules && i < 10; i++) {
    String availKey = "d" + String(i) + "-available";
    String calKey = "d" + String(i) + "-calibrated";
    shieldModuleStates[i].available = preferences.getBool(availKey.c_str(), false);
    shieldModuleStates[i].calibrated = preferences.getBool(calKey.c_str(), false);
  }

  preferences.end();

  Serial.println("Game state loaded from NVS:");
  Serial.printf("  Target: %d/%d/%d\n", targetMonth, targetDay, targetYear);
  Serial.printf("  Current: %d/%d/%d\n", currentMonth, currentDay, currentYear);
  Serial.printf("  Last: %d/%d/%d\n", lastMonth, lastDay, lastYear);
  Serial.printf("  Intercept Window: %d seconds (%02d:%02d:%02d)\n", interceptWindowSeconds,
                interceptHours, interceptMinutes, interceptSeconds);

  // Display loaded shield module states
  for (int i = 0; i < numShieldModules; i++) {
    Serial.printf("  Shield Module %d: Available=%s, Calibrated=%s\n", i,
                  shieldModuleStates[i].available ? "true" : "false",
                  shieldModuleStates[i].calibrated ? "true" : "false");
  }
}

void GameState::save() {
  preferences.begin(StorageKeys::STORAGE_NAMESPACE, false);

  // Save target date
  preferences.putUChar(StorageKeys::TARGET_MONTH, targetMonth);
  preferences.putUChar(StorageKeys::TARGET_DAY, targetDay);
  preferences.putUShort(StorageKeys::TARGET_YEAR, targetYear);

  // Save current date
  preferences.putUChar(StorageKeys::CURRENT_MONTH, currentMonth);
  preferences.putUChar(StorageKeys::CURRENT_DAY, currentDay);
  preferences.putUShort(StorageKeys::CURRENT_YEAR, currentYear);

  // Save last date
  preferences.putUChar(StorageKeys::LAST_MONTH, lastMonth);
  preferences.putUChar(StorageKeys::LAST_DAY, lastDay);
  preferences.putUShort(StorageKeys::LAST_YEAR, lastYear);

  // Save intercept window
  preferences.putInt(StorageKeys::TIME_REMAINING, interceptWindowSeconds);

  // Save number of shield modules
  preferences.putInt(StorageKeys::MODULE_COUNT, numShieldModules);

  // Save shield module states
  for (int i = 0; i < numShieldModules; i++) {
    String availKey = "d" + String(i) + "-available";
    String calKey = "d" + String(i) + "-calibrated";
    preferences.putBool(availKey.c_str(), shieldModuleStates[i].available);
    preferences.putBool(calKey.c_str(), shieldModuleStates[i].calibrated);
  }

  preferences.end();
  Serial.println("Game state saved to NVS");
}

void GameState::reset() {
  preferences.begin(StorageKeys::STORAGE_NAMESPACE, false);
  preferences.clear();
  preferences.end();
  Serial.println("NVS cleared");
}

void GameState::setTargetDate(uint8_t month, uint8_t day, uint16_t year) {
  targetMonth = month;
  targetDay = day;
  targetYear = year;
}

void GameState::getTargetDate(uint8_t& month, uint8_t& day, uint16_t& year) {
  month = targetMonth;
  day = targetDay;
  year = targetYear;
}

void GameState::setCurrentDate(uint8_t month, uint8_t day, uint16_t year) {
  currentMonth = month;
  currentDay = day;
  currentYear = year;
}

void GameState::getCurrentDate(uint8_t& month, uint8_t& day, uint16_t& year) {
  month = currentMonth;
  day = currentDay;
  year = currentYear;
}

void GameState::setLastDate(uint8_t month, uint8_t day, uint16_t year) {
  lastMonth = month;
  lastDay = day;
  lastYear = year;
}

void GameState::getLastDate(uint8_t& month, uint8_t& day, uint16_t& year) {
  month = lastMonth;
  day = lastDay;
  year = lastYear;
}

void GameState::setInterceptWindowSeconds(int seconds) {
  interceptWindowSeconds = seconds;
  updateInterceptTimeComponents();
}

int GameState::getInterceptWindowSeconds() {
  return interceptWindowSeconds;
}

void GameState::getInterceptWindowTime(int& hours, int& minutes, int& seconds) {
  hours = interceptHours;
  minutes = interceptMinutes;
  seconds = interceptSeconds;
}

void GameState::setShieldModuleState(int index, bool available, bool calibrated) {
  if (index < 0 || index >= 10)
    return;

  shieldModuleStates[index].available = available;
  shieldModuleStates[index].calibrated = calibrated;

  // Update numShieldModules if needed
  if (index >= numShieldModules) {
    numShieldModules = index + 1;
  }
}

void GameState::getShieldModuleState(int index, bool& available, bool& calibrated) {
  if (index < 0 || index >= 10) {
    available = false;
    calibrated = false;
    return;
  }

  available = shieldModuleStates[index].available;
  calibrated = shieldModuleStates[index].calibrated;
}

ModuleState* GameState::getShieldModuleStates() {
  return shieldModuleStates;
}

void GameState::rollDates() {
  // Roll the dates: last <- current <- target, and reset target
  lastMonth = currentMonth;
  lastDay = currentDay;
  lastYear = currentYear;

  currentMonth = targetMonth;
  currentDay = targetDay;
  currentYear = targetYear;

  targetMonth = 0;
  targetDay = 0;
  targetYear = 0;

  Serial.printf(
      "Dates rolled - Last: %02d/%02d/%04d, Current: %02d/%02d/%04d, Target: --/--/----\n",
      lastMonth, lastDay, lastYear, currentMonth, currentDay, currentYear);
}

void GameState::updateInterceptTimeComponents() {
  interceptHours = interceptWindowSeconds / 3600;
  interceptMinutes = (interceptWindowSeconds % 3600) / 60;
  interceptSeconds = interceptWindowSeconds % 60;
}

bool GameState::isValidTargetDate() {
  return (targetMonth != 0 && targetDay != 0 && targetYear != 0);
}
