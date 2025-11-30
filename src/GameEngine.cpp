#include "GameEngine.h"

// ============================================================================
// Public Methods
// ============================================================================
GameEngine::GameEngine(DateController& dateController) : dateController(dateController) {
  lastUpdate = 0;
  gameState.gameActive = false;
  gameState.gameStartTime = 0;
}

// Called when the game is turned on, but not yet started
void GameEngine::initialize() {
  dateController.begin();
}

// Officially starts the game
void GameEngine::startGame() {
  gameState.gameActive = true;
  gameState.gameStartTime = millis();
}

// Main game loop
void GameEngine::loop() {
  unsigned long now = millis();
  if (gameState.gameActive && (now - lastUpdate >= 1000)) {
    publishRemainingTime();
    lastUpdate = now;
  }

  dateController.loop();
}

// ============================================================================
// Private Methods
// ============================================================================

void GameEngine::publishRemainingTime() {
  GameTime remainingTime = getRemainingGameTime();

  char buffer[20];
  sprintf(buffer, "%ld,%ld,%ld", remainingTime.hours, remainingTime.minutes, remainingTime.seconds);
  Serial2.println(buffer);  // Send over UART

  Serial.printf("Remaining Time: %02d:%02d:%02d (%d%%)\n", remainingTime.hours,
                remainingTime.minutes, remainingTime.seconds, remainingTime.percentage);

  if (remainingTime.hours == 0 && remainingTime.minutes == 0 && remainingTime.seconds == 0) {
    gameState.gameActive = false;
  }
}

GameTime GameEngine::getRemainingGameTime() {
  GameTime time = {0, 0, 0, 0};

  unsigned long remainingSeconds = getRemainingGameSeconds();

  time.hours = remainingSeconds / 3600;
  remainingSeconds %= 3600;
  time.minutes = remainingSeconds / 60;
  time.seconds = remainingSeconds % 60;
  time.percentage = getRemainingTimePercentage(remainingSeconds);

  return time;
}

unsigned long GameEngine::getElapsedGameSeconds() {
  unsigned long elapsedSeconds = (millis() - gameState.gameStartTime) / 1000;
  return elapsedSeconds;
}

unsigned long GameEngine::getRemainingGameSeconds() {
  unsigned long elapsedSeconds = getElapsedGameSeconds();

  if (elapsedSeconds >= TOTAL_COUNTDOWN_SECONDS) {
    return 0;
  }

  return (TOTAL_COUNTDOWN_SECONDS - elapsedSeconds);
}

int GameEngine::getRemainingTimePercentage(unsigned long remainingSeconds) {
  return (int)((remainingSeconds * 100) / TOTAL_COUNTDOWN_SECONDS);
}
