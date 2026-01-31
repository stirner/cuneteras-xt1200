#ifndef MPU_HANDLER_H
#define MPU_HANDLER_H

#include <Adafruit_MPU6050.h>

class MPUHandler {
private:
  Adafruit_MPU6050 mpu;
  float rollFiltered;
  float rollRaw;
  float accelX;
  float accelY;
  float accelZ;
  
public:
  MPUHandler();
  
  bool begin();
  void update();
  float getRoll() const;
  float getRawRoll() const;
  float getAccelX() const;
  float getAccelY() const;
  float getAccelZ() const;
  void calibrateAccel(float ax, float ay, float az) __attribute__((unused));
  void printDiagnostics() const __attribute__((unused));
};

#endif
