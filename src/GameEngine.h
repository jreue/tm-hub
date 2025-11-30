#pragma once

#include <Arduino.h>

#include "DateController.h"

struct GameState {
    bool gameActive;
    unsigned long gameStartTime;
};

class GameEngine {
  public:
    GameEngine(DateController& dateController);
    void initialize();
    void start();
    void loop();

  private:
    DateController& dateController;
    GameState gameState;
    String currentCode;
    unsigned long lastUpdate;

    void publishRemainingTime();
};