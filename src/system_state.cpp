#include <Arduino.h>
#include "system_state.h"

SystemStateManager::SystemStateManager() 
  : currentState(STATE_INIT), previousState(STATE_INIT), stateChangeTime(millis()) {}

void SystemStateManager::setState(SystemState newState) {
  if (newState != currentState) {
    previousState = currentState;
    currentState = newState;
    stateChangeTime = millis();
    
    Serial.printf("[STATE] Transition: %s -> %s\n", getStateString(), 
                  (newState == STATE_INIT) ? "INIT" :
                  (newState == STATE_CONFIG_WINDOW) ? "CONFIG_WINDOW" :
                  (newState == STATE_NORMAL) ? "NORMAL" : "ERROR");
  }
}

unsigned long SystemStateManager::getStateElapsedTime() const {
  return millis() - stateChangeTime;
}

const char* SystemStateManager::getStateString() const {
  switch (currentState) {
    case STATE_INIT: return "INIT";
    case STATE_CONFIG_WINDOW: return "CONFIG_WINDOW";
    case STATE_NORMAL: return "NORMAL";
    case STATE_ERROR: return "ERROR";
    default: return "UNKNOWN";
  }
}

void SystemStateManager::printStateInfo() const {
  Serial.printf("[STATE INFO] Current: %s | Previous: %s | Elapsed: %lu ms\n",
    getStateString(),
    (previousState == STATE_INIT) ? "INIT" :
    (previousState == STATE_CONFIG_WINDOW) ? "CONFIG_WINDOW" :
    (previousState == STATE_NORMAL) ? "NORMAL" : "ERROR",
    getStateElapsedTime());
}
