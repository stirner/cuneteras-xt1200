#include <Arduino.h>
#include <EEPROM.h>
#include "config.h"

Config cfg;

void loadConfig() {
  EEPROM.get(EEPROM_CONFIG_ADDR, cfg);
  
  // Validación: si los valores son NaN o inválidos, usar valores por defecto
  if (isnan(cfg.angleOn) || cfg.angleOn < 0 || cfg.angleOn > 90) {
    cfg.angleOn = DEFAULT_ANGLE_ON;
  }
  if (isnan(cfg.angleOff) || cfg.angleOff < 0 || cfg.angleOff > 90) {
    cfg.angleOff = DEFAULT_ANGLE_OFF;
  }
  if (isnan(cfg.filterAlpha) || cfg.filterAlpha < 0 || cfg.filterAlpha > 1) {
    cfg.filterAlpha = DEFAULT_FILTER_ALPHA;
  }
  if (cfg.maxPWM < 0 || cfg.maxPWM > 1023) {
    cfg.maxPWM = DEFAULT_MAX_PWM;
  }
  if (cfg.fadeStep < 1 || cfg.fadeStep > 50) {
    cfg.fadeStep = DEFAULT_FADE_STEP;
  }
  
  // Validación: angleOff debe ser menor que angleOn
  if (cfg.angleOff >= cfg.angleOn) {
    cfg.angleOff = cfg.angleOn - 2.0;
  }
  
  Serial.println("Config loaded from EEPROM");
  Serial.printf("  Angle ON: %.2f°\n", cfg.angleOn);
  Serial.printf("  Angle OFF: %.2f°\n", cfg.angleOff);
  Serial.printf("  Max PWM: %d\n", cfg.maxPWM);
  Serial.printf("  Filter Alpha: %.3f\n", cfg.filterAlpha);
  Serial.printf("  Fade Step: %d\n", cfg.fadeStep);
}

void saveConfig() {
  EEPROM.put(EEPROM_CONFIG_ADDR, cfg);
  EEPROM.commit();
  Serial.println("Config saved to EEPROM");
}

void resetConfig() {
  cfg.angleOn = DEFAULT_ANGLE_ON;
  cfg.angleOff = DEFAULT_ANGLE_OFF;
  cfg.maxPWM = DEFAULT_MAX_PWM;
  cfg.filterAlpha = DEFAULT_FILTER_ALPHA;
  saveConfig();
  Serial.println("Config reset to defaults");
}
