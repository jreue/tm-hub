#include "SystemResults.h"

SystemStatus SystemResults::results[] = {{"Destination Date", "-- / -- / ----"},
                                         {"Current Date", "06 / 04 / 2055"},
                                         {"Last Departure", "06 / 04 / 2025"},
                                         {"Core Power", "ONLINE"},
                                         {"Shield Modules", "0 / 8"},
                                         {"Shield Modules Calibrated", "0 / 8"},
                                         {"Shield Status", "N/A"},
                                         {"Detection Risk", "HIGH"},
                                         {"Intercept Window", "47h 58m 32s"},
                                         {"Service Link", "DISCONNECTED"}};

const size_t SystemResults::resultCount = sizeof(SystemResults::results) / sizeof(SystemStatus);

SystemStatus* SystemResults::getResults() {
  return results;
}

size_t SystemResults::getResultCount() {
  return resultCount;
}
