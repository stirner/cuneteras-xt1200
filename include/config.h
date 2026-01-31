#ifndef CONFIG_H
#define CONFIG_H

// ============= PIN CONFIGURATION =============
#define LEFT_LED_PIN 14
#define RIGHT_LED_PIN 12

// ============= PWM CONFIGURATION =============
#define PWM_FREQ 5000
#define PWM_RES 10

// ============= EEPROM CONFIGURATION =============
#define EEPROM_SIZE 64
#define EEPROM_CONFIG_ADDR 0

// ============= WIFI CONFIGURATION =============
#define WIFI_TIMEOUT 30000
#define WIFI_SSID "CorneringLight"
#define WIFI_PASSWORD "12345678"

// ============= DEBUG CONFIGURATION =============
#define DEBUG_MODE 1  // 1 = Activo (sin MPU6050), 0 = Normal (con MPU6050)

// ============= DEFAULT CONFIGURATION VALUES =============
#define DEFAULT_ANGLE_ON 10.0
#define DEFAULT_ANGLE_OFF 7.0
#define DEFAULT_MAX_PWM 800
#define DEFAULT_FILTER_ALPHA 0.05
#define DEFAULT_FADE_STEP 5
#define DEFAULT_ACCEL_OFFSET_X 0.0
#define DEFAULT_ACCEL_OFFSET_Y 0.0
#define DEFAULT_ACCEL_OFFSET_Z 0.0

// ============= ANGLE MAPPING =============
#define ANGLE_MAP_MIN 45.0  // Ángulo máximo para el mapeo PWM
#define ANGLE_MAP_FADE_STEP 5

// ============= SENSOR CONFIGURATION =============
#define SENSOR_UPDATE_INTERVAL 10  // ms

// ============= CONFIGURATION STRUCTURE =============
struct Config {
  float angleOn = DEFAULT_ANGLE_ON;
  float angleOff = DEFAULT_ANGLE_OFF;
  int maxPWM = DEFAULT_MAX_PWM;
  float filterAlpha = DEFAULT_FILTER_ALPHA;
  int fadeStep = DEFAULT_FADE_STEP;
  float accelOffsetX = DEFAULT_ACCEL_OFFSET_X;
  float accelOffsetY = DEFAULT_ACCEL_OFFSET_Y;
  float accelOffsetZ = DEFAULT_ACCEL_OFFSET_Z;
  bool debugMode = (DEBUG_MODE == 1);
};

#endif
