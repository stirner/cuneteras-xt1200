#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

enum SystemState {
  STATE_INIT = 0,           // Inicialización
  STATE_CONFIG_WINDOW = 1,  // Ventana de configuración abierta
  STATE_NORMAL = 2,         // Operación normal
  STATE_ERROR = 3           // Estado de error
};

class SystemStateManager {
private:
  SystemState currentState;
  SystemState previousState;
  unsigned long stateChangeTime;
  
public:
  SystemStateManager();
  
  void setState(SystemState newState);
  SystemState getState() const { return currentState; }
  SystemState getPreviousState() const { return previousState; }
  unsigned long getStateElapsedTime() const;
  
  const char* getStateString() const;
  void printStateInfo() const;
};

#endif
