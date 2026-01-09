#include "SystemResults.h"

SystemStatus SystemResults::results[] = {{"Destination Date", "-- / -- / ----", TFT_YELLOW},
                                         {"Current Date", "06 / 04 / 2055", TFT_WHITE},
                                         {"Last Departure", "06 / 04 / 2025", TFT_WHITE},
                                         {"Core Power", "ONLINE", TFT_GREEN},
                                         {"Shield Modules", "0 / 8", TFT_WHITE},
                                         {"Shield Modules Calibrated", "0 / 8", TFT_WHITE},
                                         {"Shield Status", "N/A", TFT_WHITE},
                                         {"Detection Risk", "HIGH", TFT_RED},
                                         {"Intercept Window", "47h 58m 32s", TFT_ORANGE},
                                         {"Service Link", "DISCONNECTED", TFT_RED}};

const size_t SystemResults::resultCount = sizeof(SystemResults::results) / sizeof(SystemStatus);

SystemStatus* SystemResults::getResults() {
  return results;
}

size_t SystemResults::getResultCount() {
  return resultCount;
}
