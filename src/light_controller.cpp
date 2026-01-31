#include <Arduino.h>
#include "light_controller.h"
#include "config.h"
#include "config_manager.h"

LightController::LightController() 
  : leftPWM(0), rightPWM(0), leftTarget(0), rightTarget(0) {}

void LightController::begin() {
  pinMode(LEFT_LED_PIN, OUTPUT);
  pinMode(RIGHT_LED_PIN, OUTPUT);
  analogWriteRange(cfg.maxPWM);
  
  Serial.println("Light controller initialized");
}

void LightController::updateFade(int &current, int target) {
  if (current < target) {
    current += cfg.fadeStep;
    if (current > target) current = target;
  }
  else if (current > target) {
    current -= cfg.fadeStep;
    if (current < target) current = target;
  }
}

void LightController::setTarget(int left, int right) {
  leftTarget = constrain(left, 0, cfg.maxPWM);
  rightTarget = constrain(right, 0, cfg.maxPWM);
}

void LightController::updateLights(float roll) {
  // Calcular PWM objetivo basado en el ángulo de inclinación
  int newLeftTarget = 0;
  int newRightTarget = 0;
  
  if (roll > cfg.angleOn) {
    newRightTarget = map(roll, cfg.angleOn, ANGLE_MAP_MIN, 0, cfg.maxPWM);
  }
  
  if (roll < -cfg.angleOn) {
    newLeftTarget = map(-roll, cfg.angleOn, ANGLE_MAP_MIN, 0, cfg.maxPWM);
  }
  
  setTarget(newLeftTarget, newRightTarget);
  
  // Aplicar fade suave
  updateFade(leftPWM, leftTarget);
  updateFade(rightPWM, rightTarget);
  
  // Escribir a los pines PWM
  analogWrite(LEFT_LED_PIN, leftPWM);
  analogWrite(RIGHT_LED_PIN, rightPWM);
}

void LightController::printStatus() const {
  Serial.printf("[LIGHTS] Left PWM: %d/%d | Right PWM: %d/%d | Fade Step: %d\n",
    leftPWM, leftTarget, rightPWM, rightTarget, cfg.fadeStep);
}
