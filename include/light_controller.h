#ifndef LIGHT_CONTROLLER_H
#define LIGHT_CONTROLLER_H

class LightController {
private:
  int leftPWM;
  int rightPWM;
  int leftTarget;
  int rightTarget;
  
  void updateFade(int &current, int target);
  
public:
  LightController();
  
  void begin();
  void updateLights(float roll);
  void setTarget(int left, int right);
  void printStatus() const;
  
  int getLeftPWM() const { return leftPWM; }
  int getRightPWM() const { return rightPWM; }
};

#endif
