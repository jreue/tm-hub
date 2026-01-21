#pragma once

#include <Arduino.h>

struct EngineState {
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
    EngineState gameState;
    String currentCode;
    unsigned long lastUpdate;

    void publishRemainingTime();
};