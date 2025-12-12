#pragma once

#include <Arduino.h>

struct GameState {
    bool gameActive;
    unsigned long gameStartTime;
};

class GameEngine {
  public:
    GameEngine();
    void initialize();
    void start();
    void loop();

  private:
    GameState gameState;
    String currentCode;
    unsigned long lastUpdate;

    void publishRemainingTime();
};