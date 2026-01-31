#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>

#include "config.h"
#include "config_manager.h"
#include "mpu_handler.h"
#include "light_controller.h"
#include "wifi_server.h"
#include "system_state.h"

// ============= GLOBAL OBJECTS =============
MPUHandler mpu;
LightController lights;
APConfigServer wifiServer;
SystemStateManager stateManager;

// ============= STATE =============
unsigned long lastUpdateTime = 0;
unsigned long lastDebugTime = 0;

// ============= SETUP =============
void setup() {
  Serial.begin(115200);
  delay(1000);  // Tiempo para que se estabilice la comunicación serial
  
  Serial.println("\n\n=== Cornering Light System ===");
  Serial.println("Initializing...");
  
  stateManager.setState(STATE_INIT);
  
  // Inicializar EEPROM
  EEPROM.begin(EEPROM_SIZE);
  
  // Inicializar I2C y MPU6050
  Wire.begin(4, 5);  // SDA=GPIO4(D2), SCL=GPIO5(D1) para Wemos D1 mini
  
#if DEBUG_MODE
  Serial.println("[DEBUG] Modo DEBUG activo - MPU6050 deshabilitado, usando datos mock");
  // En modo debug, saltamos la inicialización del MPU6050
#else
  if (!mpu.begin()) {
    Serial.println("FATAL: MPU6050 initialization failed!");
    stateManager.setState(STATE_ERROR);
    while (1) delay(100);
  }
#endif
  
  // Inicializar controlador de luces
  lights.begin();
  
  // Inicializar WiFi y servidor
  wifiServer.begin();
  stateManager.setState(STATE_CONFIG_WINDOW);
  
  Serial.println("Setup complete. Waiting for configuration...");
}

// ============= LOOP =============
void loop() {
  
  // ============= VENTANA DE CONFIGURACIÓN =============
  if (stateManager.getState() == STATE_CONFIG_WINDOW) {
    // Manejar clientes WiFi
    wifiServer.handleClient();
    
    // Verificar si se debe cerrar la ventana de configuración
    // Criterios:
    // 1. Si hay cliente conectado: mantener abierto indefinidamente
    // 2. Si NO hay cliente: timeout de 30 segundos
    // 3. Si se guardó configuración: cerrar inmediatamente
    
    if (wifiServer.shouldCloseConfigWindow()) {
      loadConfig();
      stateManager.setState(STATE_NORMAL);
      Serial.println("Entering normal operation mode");
      return;
    }
    
    delay(10);
    return;
  }
  
  // ============= MODO NORMAL =============
  if (stateManager.getState() == STATE_NORMAL) {
    unsigned long currentTime = millis();
    
    if (currentTime - lastUpdateTime >= SENSOR_UPDATE_INTERVAL) {
      lastUpdateTime = currentTime;
      
      // Actualizar sensores
      mpu.update();
      float roll = mpu.getRoll();
      
      // Actualizar luces basado en el ángulo
      lights.updateLights(roll);
    }
    
    // Debug cada 5 segundos
    if (currentTime - lastDebugTime >= 5000) {
      lastDebugTime = currentTime;
      Serial.print("[LOOP] ");
      mpu.printDiagnostics();
      lights.printStatus();
    }
  }
  
  // ============= MODO ERROR =============
  if (stateManager.getState() == STATE_ERROR) {
    delay(1000);
    return;
  }
  
  delay(1);
}
