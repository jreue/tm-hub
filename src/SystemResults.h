#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>

struct SystemStatus {
    String label;
    String value;
    uint16_t color;
};

class SystemResults {
  public:
    static SystemStatus results[];
    static const size_t resultCount;

    static SystemStatus* getResults();
    static size_t getResultCount();

    // Indices for easy access to specific results
    static const int IDX_DESTINATION_DATE = 0;
    static const int IDX_CURRENT_DATE = 1;
    static const int IDX_LAST_DEPARTURE = 2;
    static const int IDX_CORE_POWER = 3;
    static const int IDX_SHIELD_MODULES = 4;
    static const int IDX_SHIELD_MODULES_CALIBRATED = 5;
    static const int IDX_SHIELD_STATUS = 6;
    static const int IDX_DETECTION_RISK = 7;
    static const int IDX_INTERCEPT_WINDOW = 8;
    static const int IDX_SERVICE_LINK = 9;
};
