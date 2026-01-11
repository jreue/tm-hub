#pragma once

// ====================
// I2C Configuration
// ====================
#define I2C_SDA_PIN 21  // GREEN
#define I2C_SCL_PIN 19  // YELLOW

// Hub I2C Address
#define DATE_I2C_ADDRESS 0x22

// Known Device Module ESP-NOW IDs
#define DEVICE_1_ID 101
#define DEVICE_2_ID 102
#define DEVICE_3_ID 103

// LED Status Display
#define LED_DATA_PIN 23
#define NUM_LEDS 48
#define MATRIX_COLS 4
#define MATRIX_ROWS 12

// ====================
// ESP-NOW Configuration
// ====================
#define SCANNER_MAC_ADDRESS {0x44, 0x1D, 0x64, 0xF7, 0x4C, 0xEC}