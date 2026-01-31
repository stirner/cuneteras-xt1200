#include <Arduino.h>
#include <Wire.h>
#include "mpu_handler.h"
#include "config.h"
#include "config_manager.h"

MPUHandler::MPUHandler() : rollFiltered(0), rollRaw(0), accelX(0), accelY(0), accelZ(0) {}

bool MPUHandler::begin() {
  if (!mpu.begin()) {
    Serial.println("Failed to initialize MPU6050!");
    return false;
  }
  
  Serial.println("MPU6050 initialized successfully");
  return true;
}

void MPUHandler::update() {
#if DEBUG_MODE
  // En modo debug, generar datos oscillantes para testing
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();
  
  if (now - lastUpdate >= 100) {  // Actualizar cada 100ms
    lastUpdate = now;
    // Oscilar entre -30° y +30° cada 4 segundos
    rollRaw = 30.0 * sin(now / 2000.0 * 3.14159);
    rollFiltered = cfg.filterAlpha * rollRaw + (1.0 - cfg.filterAlpha) * rollFiltered;
    accelX = sin(now / 2000.0 * 3.14159) * 5;
    accelY = cos(now / 2000.0 * 3.14159) * 5;
    accelZ = 9.8;
  }
#else
  sensors_event_t a, g, t;
  mpu.getEvent(&a, &g, &t);
  
  // Guardar valores crudos sin calibración
  accelX = a.acceleration.x;
  accelY = a.acceleration.y;
  accelZ = a.acceleration.z;
  
  // Aplicar offset de calibración
  float ax = accelX - cfg.accelOffsetX;
  float ay = accelY - cfg.accelOffsetY;
  float az = accelZ - cfg.accelOffsetZ;
  
  // Calcular roll a partir de aceleración
  float rawRoll = atan2(ay, az) * 57.2958;  // radianes a grados
  
  // Limitar a ±90° para evitar glitches
  rawRoll = constrain(rawRoll, -90.0, 90.0);
  rollRaw = rawRoll;
  
  // Aplicar filtro exponencial
  rollFiltered = cfg.filterAlpha * rawRoll + (1.0 - cfg.filterAlpha) * rollFiltered;
#endif
}

float MPUHandler::getRoll() const {
  return rollFiltered;
}

float MPUHandler::getRawRoll() const {
  return rollRaw;
}

float MPUHandler::getAccelX() const {
  return accelX;
}

float MPUHandler::getAccelY() const {
  return accelY;
}

float MPUHandler::getAccelZ() const {
  return accelZ;
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
