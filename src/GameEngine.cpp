#include "GameEngine.h"

#include "GameTimeUtils.h"

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
void GameEngine::start() {
  gameState.gameActive = true;
  gameState.gameStartTime = millis();
}

// Main game loop
void GameEngine::loop() {
  unsigned long now = millis();
  if (gameState.gameActive && (now - lastUpdate >= 1000)) {
  //  publishRemainingTime();
    lastUpdate = now;
  }

  dateController.loop();
}

// ============================================================================
// Private Methods
// ============================================================================

void GameEngine::publishRemainingTime() {
  GameTime remainingTime = GameTimeUtils::getRemainingGameTime(gameState.gameStartTime);

  char buffer[20];
  sprintf(buffer, "%d,%d,%d", remainingTime.hours, remainingTime.minutes, remainingTime.seconds);
  Serial2.println(buffer);  // Send over UART

  Serial.printf("Remaining Time: %02d:%02d:%02d (%d%%)\n", remainingTime.hours,
                remainingTime.minutes, remainingTime.seconds, remainingTime.percentage);

  if (remainingTime.hours == 0 && remainingTime.minutes == 0 && remainingTime.seconds == 0) {
    gameState.gameActive = false;
  }
}
