#include "I2CBusManager.h"

#include <Wire.h>

constexpr uint8_t I2CBusManager::DEVICE_ADDRESSES[];

I2CBusManager::I2CBusManager() {
}

std::vector<uint8_t> I2CBusManager::scanBus() {
  std::vector<uint8_t> foundAddresses;

  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    uint8_t error = Wire.endTransmission();
    if (error == 0) {
      foundAddresses.push_back(addr);
    }
  }

  return foundAddresses;
}

bool I2CBusManager::isDeviceAvailable(uint8_t addr) {
  Wire.beginTransmission(addr);
  uint8_t error = Wire.endTransmission();
  return (error == 0);
}

bool I2CBusManager::readDeviceCalibrationStatus(uint8_t address) {
  Wire.requestFrom(address, (uint8_t)1);
  if (Wire.available()) {
    return Wire.read();
  }
  return false;
}

std::vector<DeviceStateChange> I2CBusManager::checkAllDevices() {
  std::vector<DeviceStateChange> changes;

  for (int i = 0; i < NUM_DEVICES; i++) {
    checkDevice(DEVICE_ADDRESSES[i], i, changes);
  }

  return changes;
}

void I2CBusManager::checkDevice(uint8_t address, int displayRow,
                                std::vector<DeviceStateChange>& changes) {
  bool isAvailable = isDeviceAvailable(address);
  bool isCalibrated = false;

  if (isAvailable) {
    isCalibrated = readDeviceCalibrationStatus(address);
  }

  DeviceState& prevState = deviceStates_[displayRow];
  bool availabilityChanged = (prevState.available != isAvailable);
  bool calibrationChanged = (prevState.calibrated != isCalibrated);

  if (availabilityChanged || calibrationChanged) {
    prevState.available = isAvailable;
    prevState.calibrated = isCalibrated;

    DeviceStateChange change;
    change.address = address;
    change.index = displayRow;
    change.available = isAvailable;
    change.calibrated = isCalibrated;
    change.availabilityChanged = availabilityChanged;
    change.calibrationChanged = calibrationChanged;

    changes.push_back(change);
  }
}
