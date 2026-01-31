#include <Arduino.h>
#include <Wire.h>
#include "mpu_handler.h"
#include "config_manager.h"

MPUHandler::MPUHandler() : rollFiltered(0), rollRaw(0) {}

bool MPUHandler::begin() {
  if (!mpu.begin()) {
    Serial.println("Failed to initialize MPU6050!");
    return false;
  }
  
  Serial.println("MPU6050 initialized successfully");
  return true;
}

void MPUHandler::update() {
  sensors_event_t a, g, t;
  mpu.getEvent(&a, &g, &t);
  
  // Aplicar offset de calibración
  float ax = a.acceleration.x - cfg.accelOffsetX;
  float ay = a.acceleration.y - cfg.accelOffsetY;
  float az = a.acceleration.z - cfg.accelOffsetZ;
  
  // Calcular roll a partir de aceleración
  float rawRoll = atan2(ay, az) * 57.2958;  // radianes a grados
  
  // Limitar a ±90° para evitar glitches
  rawRoll = constrain(rawRoll, -90.0, 90.0);
  rollRaw = rawRoll;
  
  // Aplicar filtro exponencial
  rollFiltered = cfg.filterAlpha * rawRoll + (1.0 - cfg.filterAlpha) * rollFiltered;
}

float MPUHandler::getRoll() const {
  return rollFiltered;
}

float MPUHandler::getRawRoll() const {
  return rollRaw;
}

void MPUHandler::calibrateAccel(float ax, float ay, float az) {
  cfg.accelOffsetX = ax;
  cfg.accelOffsetY = ay;
  cfg.accelOffsetZ = az;
  Serial.printf("Accel calibrated: X=%.3f Y=%.3f Z=%.3f\n", ax, ay, az);
}

void MPUHandler::printDiagnostics() const {
  Serial.printf("[MPU] Raw Roll: %.2f° | Filtered Roll: %.2f° | Offsets: X=%.3f Y=%.3f Z=%.3f\n",
    rollRaw, rollFiltered, cfg.accelOffsetX, cfg.accelOffsetY, cfg.accelOffsetZ);
}
