#pragma once

#include <Arduino.h>

#include <vector>

#include "hardware_config.h"

struct DeviceState {
    bool available = false;
    bool calibrated = false;
};

struct DeviceStateChange {
    uint8_t address;
    int index;
    bool available;
    bool calibrated;
    bool availabilityChanged;
    bool calibrationChanged;
};

class I2CBusManager {
  public:
    I2CBusManager();

    std::vector<uint8_t> scanBus();
    bool isDeviceAvailable(uint8_t addr);
    std::vector<DeviceStateChange> checkAllDevices();

    static constexpr uint8_t DEVICE_ADDRESSES[] = {DEVICE_1_ADDR, DEVICE_2_ADDR, DEVICE_3_ADDR};
    static constexpr int NUM_DEVICES = sizeof(DEVICE_ADDRESSES) / sizeof(DEVICE_ADDRESSES[0]);

  private:
    DeviceState deviceStates_[NUM_DEVICES];

    bool readDeviceCalibrationStatus(uint8_t address);
    void checkDevice(uint8_t address, int displayRow, std::vector<DeviceStateChange>& changes);
};
