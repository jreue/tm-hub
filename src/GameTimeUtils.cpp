#include "GameTimeUtils.h"

#include <Arduino.h>

GameTime GameTimeUtils::getRemainingGameTime(unsigned long gameStartTime) {
  GameTime time = {0, 0, 0, 0};
  unsigned long remainingSeconds = getRemainingGameSeconds(gameStartTime);
  time.hours = remainingSeconds / 3600;
  remainingSeconds %= 3600;
  time.minutes = remainingSeconds / 60;
  time.seconds = remainingSeconds % 60;
  time.percentage = getRemainingTimePercentage(remainingSeconds);
  return time;
}

unsigned long GameTimeUtils::getElapsedGameSeconds(unsigned long gameStartTime) {
  return (millis() - gameStartTime) / 1000;
}

unsigned long GameTimeUtils::getRemainingGameSeconds(unsigned long gameStartTime) {
  unsigned long elapsedSeconds = getElapsedGameSeconds(gameStartTime);
  if (elapsedSeconds >= TOTAL_COUNTDOWN_SECONDS) {
    return 0;
  }
  return (TOTAL_COUNTDOWN_SECONDS - elapsedSeconds);
}

int GameTimeUtils::getRemainingTimePercentage(unsigned long remainingSeconds) {
  return (int)((remainingSeconds * 100) / TOTAL_COUNTDOWN_SECONDS);
}
