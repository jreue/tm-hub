#pragma once

#include "GameEngine.h"
#include "game_config.h"

struct GameTime {
    int hours;
    int minutes;
    int seconds;
    int percentage;
};

namespace GameTimeUtils {
GameTime getRemainingGameTime(unsigned long gameStartTime);
unsigned long getElapsedGameSeconds(unsigned long gameStartTime);
unsigned long getRemainingGameSeconds(unsigned long gameStartTime);
int getRemainingTimePercentage(unsigned long remainingSeconds);
}  // namespace GameTimeUtils
