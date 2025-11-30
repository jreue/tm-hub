#pragma once

#include <Arduino.h>

#include "DateController.h"

#define COUNTDOWN_MINUTES 2
#define TOTAL_COUNTDOWN_SECONDS ((unsigned long)COUNTDOWN_MINUTES * 60)

struct GameTime {
    int hours;
    int minutes;
    int seconds;
    int percentage;
};

struct GameState {
    bool gameActive;
    unsigned long gameStartTime;
};

class GameEngine {
  public:
    GameEngine(DateController& dateController);
    void initialize();
    void startGame();
    void loop();

  private:
    DateController& dateController;
    GameState gameState;
    String currentCode;
    unsigned long lastUpdate;

    void publishRemainingTime();

    GameTime getRemainingGameTime();
    unsigned long getElapsedGameSeconds();
    unsigned long getRemainingGameSeconds();
    int getRemainingTimePercentage(unsigned long remainingSeconds);
};