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
    // Date-related results
    static SystemStatus dateResults[];
    static const size_t dateResultCount;
    static const int IDX_DATE_DESTINATION = 0;
    static const int IDX_DATE_CURRENT = 1;
    static const int IDX_DATE_LAST_DEPARTURE = 2;

    // Module-related results
    static SystemStatus moduleResults[];
    static const size_t moduleResultCount;
    static const int IDX_MODULE_SHIELD_MODULES = 0;
    static const int IDX_MODULE_SHIELD_CALIBRATED = 1;
    static const int IDX_MODULE_SHIELD_STATUS = 2;

    // Risk/Intercept results
    static SystemStatus riskResults[];
    static const size_t riskResultCount;
    static const int IDX_RISK_DETECTION_RISK = 0;
    static const int IDX_RISK_INTERCEPT_WINDOW = 1;

    // General system results
    static SystemStatus systemResults[];
    static const size_t systemResultCount;
    static const int IDX_SYSTEM_CORE_POWER = 0;
    static const int IDX_SYSTEM_SERVICE_LINK = 1;
};
