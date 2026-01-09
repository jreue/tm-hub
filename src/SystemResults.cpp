#include "SystemResults.h"

// Date-related results
SystemStatus SystemResults::dateResults[] = {{"Destination Date", "-- / -- / ----", TFT_YELLOW},
                                             {"Current Date", "06 / 04 / 2055", TFT_WHITE},
                                             {"Last Departure", "06 / 04 / 2025", TFT_WHITE}};

const size_t SystemResults::dateResultCount =
    sizeof(SystemResults::dateResults) / sizeof(SystemStatus);

// Module-related results
SystemStatus SystemResults::moduleResults[] = {{"Shield Modules", "0 / 8", TFT_WHITE},
                                               {"Shield Modules Calibrated", "0 / 8", TFT_WHITE},
                                               {"Shield Status", "N/A", TFT_WHITE}};

const size_t SystemResults::moduleResultCount =
    sizeof(SystemResults::moduleResults) / sizeof(SystemStatus);

// Risk/Intercept results
SystemStatus SystemResults::riskResults[] = {{"Detection Risk", "HIGH", TFT_RED},
                                             {"Intercept Window", "47h 58m 32s", TFT_ORANGE}};

const size_t SystemResults::riskResultCount =
    sizeof(SystemResults::riskResults) / sizeof(SystemStatus);

// General system results
SystemStatus SystemResults::systemResults[] = {{"Core Power", "ONLINE", TFT_GREEN},
                                               {"Service Link", "DISCONNECTED", TFT_RED}};

const size_t SystemResults::systemResultCount =
    sizeof(SystemResults::systemResults) / sizeof(SystemStatus);
