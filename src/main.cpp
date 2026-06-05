#include <Arduino.h>
#include <Button.h>
#include <EspNowHelper.h>
#include <MessageStructs.h>
#include <shared_hardware_config.h>

#include "DisplayController.h"
#include "GameEngine.h"
#include "GameState.h"
#include "LEDStatusHelper.h"
#include "hardware_config.h"

EspNowHelper espNowHelper;
LEDStatusHelper ledHelper;
DisplayController displayController;
GameState gameState;
GameEngine gameEngine;

const uint8_t KNOWN_MODULE_IDS[] = {MODULE_1_ID, MODULE_2_ID, MODULE_3_ID,
                                    MODULE_4_ID, MODULE_5_ID, MODULE_6_ID};
const int NUM_MODULES = sizeof(KNOWN_MODULE_IDS) / sizeof(KNOWN_MODULE_IDS[0]);

// ESP-NOW message flags (volatile because accessed in ISR)
volatile bool dateMessagePending = false;
DateMessage pendingDateMessage;

// Timer interrupt flag
volatile bool interceptTickFlag = false;

// Deferred game-complete trigger — set when all modules calibrate,
// but handleGameComplete() is not called until the LED effect finishes
bool gameCompletePending = false;

// Helper to get shield module index from ID
int getShieldModuleIndex(uint8_t moduleId) {
  for (int i = 0; i < NUM_MODULES; i++) {
    if (KNOWN_MODULE_IDS[i] == moduleId) {
      return i;
    }
  }
  return -1;  // Not found
}

uint8_t scannerMacAddress[] = SCANNER_MAC_ADDRESS;
uint8_t dateMacAddress[] = DATE_MAC_ADDRESS;
uint8_t effectsMacAddress[] = EFFECTS_MAC_ADDRESS;

// Timer Interrupt Handler
void IRAM_ATTR handleTimerInterrupt();

// Date Message Handlers
void processPendingDateMessage();
void handleDateChanged(const DateMessage& msg);

// Scanner Message Handlers
void handleScannerMessage(const ScannerMessage& msg);
void handleScannerConnected();

// Shield Module Message Handlers
void handleShieldModuleMessage(const ShieldModuleMessage& msg);
void handleShieldModuleOnline(int moduleIndex, uint8_t moduleId, bool calibrated);
void handleShieldModuleCalibrationChanged(int moduleIndex, uint8_t moduleId, bool calibrated);

void handleDevice858Message(const Device858Message& msg);

void setupButtons();

void updateDateStateOnHubDisplay();
void updateShieldModuleStateOnHubDisplay();
void updateShieldModuleStateOnHubLeds();
void updateShieldModuleStateOnScanner(ShieldModuleMessage msg);

// Button Callbacks
void handleTravelButtonPress(void* button_handle, void* usr_data);
void handleResetButtonPress(void* button_handle, void* usr_data);

void logKnownShieldModules();

void updateDateStateOnHubDisplay();

void handleInterceptTick();

void handleGameComplete();
void checkAndHandleGameComplete();

void forwardScannerConnectedToEffects();
void forwardShieldModuleStateToEffects(ShieldModuleMessage msg);
void forwardTravelEventToEffects(uint8_t month, uint8_t day, uint16_t year);

hw_timer_t* timer = NULL;

void setup() {
  Serial.begin(115200);
  Serial.println("HUB Starting...");

  timer = timerBegin(0, 80, true);  // Timer 0, prescaler 80 (1us per tick)
  timerAttachInterrupt(timer, &handleTimerInterrupt, true);
  timerAlarmWrite(timer, 1000000, true);  // 1 second alarm
  timerAlarmEnable(timer);

  logKnownShieldModules();
  setupButtons();

  gameState.begin();
  gameState.load();

  ledHelper.begin();

  displayController.begin(NUM_MODULES);

  espNowHelper.begin(DEVICE_ID);
  espNowHelper.addPeer(scannerMacAddress);
  espNowHelper.addPeer(dateMacAddress);
  espNowHelper.addPeer(effectsMacAddress);
  espNowHelper.registerDateMessageHandler(handleDateChanged);
  espNowHelper.registerScannerMessageHandler(handleScannerMessage);
  espNowHelper.registerModuleMessageHandler(handleShieldModuleMessage);
  espNowHelper.registerDevice858MessageHandler(handleDevice858Message);

  // Restore states from loaded data
  updateDateStateOnHubDisplay();
  updateShieldModuleStateOnHubDisplay();
  updateShieldModuleStateOnHubLeds();

  int h, m, s;
  gameState.getInterceptWindowTime(h, m, s);
  displayController.updateInterceptWindow(h, m, s, true, true);
}

void loop() {
  processPendingDateMessage();

  ledHelper.animate();

  handleInterceptTick();

  // Wait for the calibration celebration LED effect to finish before
  // triggering the TFT game-complete sequence
  if (gameCompletePending && !ledHelper.isEffectActive()) {
    gameCompletePending = false;
    handleGameComplete();
  }

  if (gameState.isGameComplete()) {
    displayController.animateShieldingCompleteEffect();
    // Fire the effects-device message and start the disco LED show in the last 8 seconds of the
    // countdown
    if (displayController.shouldFireFinalEffect()) {
      espNowHelper.sendHubEffect(effectsMacAddress, 6);
      ledHelper.triggerDiscoPartyEffect();
    }
  }

  delay(10);  // Small delay to avoid busy loop
}

void logKnownShieldModules() {
  Serial.print("Known Shield Module IDs: [");
  for (int i = 0; i < NUM_MODULES; i++) {
    Serial.printf("%d", KNOWN_MODULE_IDS[i]);
    if (i < NUM_MODULES - 1)
      Serial.print(", ");
  }
  Serial.println("]");
}

void setupButtons() {
  Button* travelButton = new Button(TRAVEL_BUTTON_PIN, false);
  travelButton->attachSingleClickEventCb(&handleTravelButtonPress, NULL);

  Button* resetButton = new Button(RESET_BUTTON_PIN, false);
  resetButton->attachSingleClickEventCb(&handleResetButtonPress, NULL);
}

void IRAM_ATTR handleTimerInterrupt() {
  interceptTickFlag = true;
}

void processPendingDateMessage() {
  // Process pending ESP-NOW messages first
  if (dateMessagePending) {
    dateMessagePending = false;
    // Store the incoming date as the target date
    gameState.setTargetDate(pendingDateMessage.month, pendingDateMessage.day,
                            pendingDateMessage.year);
    displayController.updateTargetDate(pendingDateMessage.month, pendingDateMessage.day,
                                       pendingDateMessage.year);
  }
}

void handleDateChanged(const DateMessage& msg) {
  Serial.printf("Date Update Received: %02d/%02d/%04d\n", msg.month, msg.day, msg.year);
  // Don't update display from ISR context - set flag instead
  pendingDateMessage = msg;
  dateMessagePending = true;
}

void handleScannerMessage(const ScannerMessage& msg) {
  Serial.printf("Handling scanner message type: %d\n", msg.messageType);

  switch (msg.messageType) {
    case MSG_TYPE_CONNECT:
      handleScannerConnected();
      break;
    case MSG_TYPE_DATA:
      Serial.println("Received data message from scanner. Nothing to do yet.");
      break;
    default:
      Serial.println("Unknown scanner message type.");
      return;
  }
}

void handleScannerConnected() {
  Serial.println("Scanner has connected!");
  displayController.updateServiceLink(true);

  forwardScannerConnectedToEffects();
}

void handleInterceptTick() {
  if (interceptTickFlag) {
    interceptTickFlag = false;

    if (gameState.isGameComplete()) {
      return;  // Game complete — freeze the timer display
    }

    if (!gameState.isGameStarted()) {
      return;  // Game not started yet — freeze the timer display
    }

    gameState.tickCountdown();

    int hours, minutes, seconds;
    gameState.getInterceptWindowTime(hours, minutes, seconds);

    if (hours == 0 && minutes == 0 && seconds == 0) {
      return;
    }

    // Track changes for display optimization
    // Static variables: persist across calls but scoped only to this function
    static int lastHours = -1;
    static int lastMinutes = -1;

    bool hoursChanged = (hours != lastHours);
    bool minutesChanged = (minutes != lastMinutes);

    lastHours = hours;
    lastMinutes = minutes;

    // Update display
    displayController.updateInterceptWindow(hours, minutes, seconds, hoursChanged, minutesChanged);

    // Save when minutes change
    if (minutesChanged) {
      gameState.save();
    }
  }
}

void handleShieldModuleMessage(const ShieldModuleMessage& msg) {
  Serial.printf("Handling shield module message from ID: %d\n", msg.deviceId);

  int moduleIndex = getShieldModuleIndex(msg.deviceId);
  if (moduleIndex < 0) {
    Serial.printf("Unknown shield module ID: %d\n", msg.deviceId);
    return;
  }

  bool wasAvailable, wasCalibrated;
  gameState.getShieldModuleState(moduleIndex, wasAvailable, wasCalibrated);

  // Handle different message types
  switch (msg.messageType) {
    case MSG_TYPE_CONNECT:
      gameState.setShieldModuleState(moduleIndex, true, msg.isCalibrated);
      handleShieldModuleOnline(moduleIndex, msg.deviceId, msg.isCalibrated);
      break;
    case MSG_TYPE_DATA:
      gameState.setShieldModuleState(moduleIndex, true, msg.isCalibrated);

      // Only trigger handlers if state actually changed
      if (!wasAvailable) {
        handleShieldModuleOnline(moduleIndex, msg.deviceId, msg.isCalibrated);
      } else if (wasCalibrated != msg.isCalibrated) {
        handleShieldModuleCalibrationChanged(moduleIndex, msg.deviceId, msg.isCalibrated);
      }
      break;
    default:
      Serial.println("Unknown shield module message type.");
  }

  updateShieldModuleStateOnScanner(msg);
  forwardShieldModuleStateToEffects(msg);
}

void handleShieldModuleOnline(int moduleIndex, uint8_t moduleId, bool calibrated) {
  Serial.printf("Shield Module (%d): ONLINE - Calibrated: %s\n", moduleId,
                calibrated ? "TRUE" : "FALSE");

  ledHelper.updateStatusLEDs(moduleIndex, true, calibrated);
  updateShieldModuleStateOnHubDisplay();
  gameState.save();
  checkAndHandleGameComplete();
}

void handleShieldModuleCalibrationChanged(int moduleIndex, uint8_t moduleId, bool calibrated) {
  Serial.printf("Shield Module (%d): Changed to %s\n", moduleId, calibrated ? "TRUE" : "FALSE");

  ledHelper.updateStatusLEDs(moduleIndex, true, calibrated);
  updateShieldModuleStateOnHubDisplay();
  gameState.save();
  checkAndHandleGameComplete();
}

void handleDevice858Message(const Device858Message& msg) {
  Serial.printf("Handling 858 message from ID: %d\n", msg.deviceId);

  switch (msg.messageType) {
    case MSG_TYPE_CONNECT:
      Serial.println("858 device connected.");  // not sent by 858 yet, but handle just in case
      break;
    case MSG_TYPE_DATA:
      Serial.println("Received data message from 858 device. Nothing to do yet.");

      if (msg.doTravelOverride) {
        Serial.println("Travel override flag is set in 858 message. Triggering travel event...");

        handleTravelButtonPress(nullptr, nullptr);
      } else if (msg.doResetOverride) {
        Serial.println("Reset override flag is set in 858 message. Triggering reset event...");
        handleResetButtonPress(nullptr, nullptr);
      } else if (msg.doStartupOverride) {
        Serial.println(
            "Startup override flag is set in 858 message. No startup behavior defined yet.");
      }
      break;
    default:
      Serial.println("Unknown 858 message type.");
  }
}

void handleTravelButtonPress(void* button_handle, void* usr_data) {
  Serial.println(">>> Travel button pressed! <<<");

  // Check if target date is valid (not --/--/----)
  if (!gameState.isValidTargetDate()) {
    Serial.println("Cannot travel: No valid target date set.");
    return;
  }

  gameState.startGame();
  ledHelper.triggerTravelEffect();

  // Roll the dates
  gameState.rollDates();

  // Get dates for display update
  uint8_t targetMonth, targetDay, currentMonth, currentDay, lastMonth, lastDay;
  uint16_t targetYear, currentYear, lastYear;
  gameState.getTargetDate(targetMonth, targetDay, targetYear);
  gameState.getCurrentDate(currentMonth, currentDay, currentYear);
  gameState.getLastDate(lastMonth, lastDay, lastYear);

  // Update the display with the new dates
  displayController.updateLastDeparture(lastMonth, lastDay, lastYear);
  displayController.updateCurrentDate(currentMonth, currentDay, currentYear);
  displayController.updateTargetDate(targetMonth, targetDay, targetYear);

  gameState.save();

  // Send current date to scanner for date validation during environment scanning
  espNowHelper.sendDateUpdated(scannerMacAddress, currentMonth, currentDay, currentYear);
  // clear target date after travel
  espNowHelper.sendDateUpdated(dateMacAddress, targetMonth, targetDay, targetYear);

  forwardTravelEventToEffects(targetMonth, targetDay, targetYear);
}

void handleResetButtonPress(void* button_handle, void* usr_data) {
  Serial.println(">>> Reset button pressed! Clearing NVS... <<<");
  gameState.reset();
  Serial.println("NVS cleared. Restarting...");
  ESP.restart();
}

void updateDateStateOnHubDisplay() {
  uint8_t targetMonth, targetDay, currentMonth, currentDay, lastMonth, lastDay;
  uint16_t targetYear, currentYear, lastYear;
  gameState.getTargetDate(targetMonth, targetDay, targetYear);
  gameState.getCurrentDate(currentMonth, currentDay, currentYear);
  gameState.getLastDate(lastMonth, lastDay, lastYear);

  displayController.updateTargetDate(targetMonth, targetDay, targetYear);
  displayController.updateCurrentDate(currentMonth, currentDay, currentYear);
  displayController.updateLastDeparture(lastMonth, lastDay, lastYear);
}

void initShieldModules() {
  for (int i = 0; i < NUM_MODULES; i++) {
    gameState.setShieldModuleState(i, false, false);
    ledHelper.updateStatusLEDs(i, false, false);
  }
  updateShieldModuleStateOnHubDisplay();
}

void updateShieldModuleStateOnHubDisplay() {
  int onlineCount = 0;
  int calibratedCount = 0;

  ModuleState* moduleStates = gameState.getShieldModuleStates();
  for (int i = 0; i < NUM_MODULES; i++) {
    if (moduleStates[i].available) {
      onlineCount++;
      if (moduleStates[i].calibrated) {
        calibratedCount++;
      }
    }
  }

  displayController.updateShieldModules(onlineCount, calibratedCount);
}

void updateShieldModuleStateOnHubLeds() {
  ModuleState* moduleStates = gameState.getShieldModuleStates();
  for (int i = 0; i < NUM_MODULES; i++) {
    ledHelper.updateStatusLEDs(i, moduleStates[i].available, moduleStates[i].calibrated);
  }
}

void updateShieldModuleStateOnScanner(ShieldModuleMessage msg) {
  Serial.println("Sending shield module state update to scanner...");

  esp_now_send(scannerMacAddress, (uint8_t*)&msg, sizeof(msg));
}

void forwardScannerConnectedToEffects() {
  Serial.println("Forwarding scanner connected event to effects...");

  espNowHelper.sendScannerConnected(effectsMacAddress);
}

void forwardShieldModuleStateToEffects(ShieldModuleMessage msg) {
  Serial.println("Forwarding shield module state update to effects...");

  switch (msg.messageType) {
    case MSG_TYPE_CONNECT:
      espNowHelper.sendModuleConnected(effectsMacAddress);
      break;
    case MSG_TYPE_DATA:
      espNowHelper.sendModuleUpdated(effectsMacAddress, msg.isCalibrated);
      break;
    default:
      Serial.println("Unknown shield module message type for forwarding.");
      return;
  }
}

void forwardTravelEventToEffects(uint8_t month, uint8_t day, uint16_t year) {
  Serial.println("Forwarding travel event to effects...");

  espNowHelper.sendDateUpdated(effectsMacAddress, month, day, year);
}

void checkAndHandleGameComplete() {
  if (gameState.isGameComplete()) {
    return;  // Already triggered
  }

  ModuleState* moduleStates = gameState.getShieldModuleStates();
  int calibratedCount = 0;
  for (int i = 0; i < NUM_MODULES; i++) {
    if (moduleStates[i].calibrated) {
      calibratedCount++;
    }
  }

  if (calibratedCount == NUM_MODULES) {
    gameState.setGameComplete(true);
    gameState.save();
    gameCompletePending = true;  // Defer until LED celebration effect finishes
  }
}

void handleGameComplete() {
  Serial.println("*** GAME COMPLETE! All shield modules calibrated! ***");
  // Timer countdown is frozen via the guard in handleInterceptTick()
  displayController.triggerShieldingCompleteEffect();
  // espNowHelper.sendHubEffect fires later, via shouldFireFinalEffect() in loop()
}