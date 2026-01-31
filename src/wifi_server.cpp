#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "wifi_server.h"
#include "config.h"
#include "config_manager.h"
#include "mpu_handler.h"
#include "light_controller.h"

// HTML almacenado en PROGMEM para ahorrar RAM
const char HTML_ROOT[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Cornering Light Config</title>
  <style>
    body { font-family: Arial; margin: 20px; background: #f5f5f5; }
    .container { max-width: 500px; margin: 0 auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
    h2 { color: #333; }
    .form-group { margin: 15px 0; }
    label { display: block; font-weight: bold; margin-bottom: 5px; color: #555; }
    input { width: 100%; padding: 8px; border: 1px solid #ddd; border-radius: 4px; box-sizing: border-box; }
    button { width: 100%; padding: 10px; margin-top: 10px; border: none; border-radius: 4px; cursor: pointer; font-weight: bold; }
    .btn-save { background-color: #4CAF50; color: white; }
    .btn-save:hover { background-color: #45a049; }
    .btn-reset { background-color: #f44336; color: white; }
    .btn-reset:hover { background-color: #da190b; }
    .btn-debug { background-color: #2196F3; color: white; }
    .btn-debug:hover { background-color: #0b7dda; }
    .button-group { display: flex; gap: 10px; }
    .button-group button { flex: 1; }
    hr { margin: 20px 0; border: none; border-top: 2px solid #eee; }
    .info { background: #e3f2fd; padding: 10px; border-radius: 4px; margin-top: 10px; font-size: 12px; }
  </style>
</head>
<body>
  <div class="container">
    <h2>[CONFIG] Cornering Light Configuration</h2>
    <form action="/save" method="GET">
      <div class="form-group">
        <label>Angle ON (degrees):</label>
        <input type="number" name="aon" step="0.1" min="0" max="90" value="%ANGLE_ON%" required>
      </div>
      <div class="form-group">
        <label>Angle OFF (degrees):</label>
        <input type="number" name="aoff" step="0.1" min="0" max="90" value="%ANGLE_OFF%" required>
      </div>
      <div class="form-group">
        <label>Max PWM (0-1023):</label>
        <input type="number" name="pwm" min="0" max="1023" value="%MAX_PWM%" required>
      </div>
      <div class="form-group">
        <label>Filter Alpha (0-1):</label>
        <input type="number" name="alpha" step="0.01" min="0" max="1" value="%FILTER_ALPHA%" required>
      </div>
      <div class="form-group">
        <label>Fade Step (1-50):</label>
        <input type="number" name="fade" min="1" max="50" value="%FADE_STEP%" required>
      </div>
      <div class="form-group">
        <label>
          <input type="checkbox" name="debug" value="1" %DEBUG_CHECKED%>
          Debug Mode (Simulated Sensor)
        </label>
      </div>
      <hr>
      <h3 style="color: #666;">MPU6050 Calibration Offsets</h3>
      <div class="form-group">
        <label>Accel Offset X:</label>
        <input type="number" name="ox" step="0.001" value="%OFFSET_X%" required>
      </div>
      <div class="form-group">
        <label>Accel Offset Y:</label>
        <input type="number" name="oy" step="0.001" value="%OFFSET_Y%" required>
      </div>
      <div class="form-group">
        <label>Accel Offset Z:</label>
        <input type="number" name="oz" step="0.001" value="%OFFSET_Z%" required>
      </div>
      <button type="submit" class="btn-save">[SAVE] Save Configuration</button>
    </form>
    <div class="button-group">
      <button onclick="location.href='/reset'" class="btn-reset">[RST] Reset to Defaults</button>
      <button onclick="location.href='/debug'" class="btn-debug">[DBG] Debug Info</button>
    </div>
    <div class="info">[i] Current values will be loaded from device on refresh</div>
  </div>
</body>
</html>
)rawliteral";

const char HTML_SAVED[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head><meta charset="UTF-8"><title>Saved</title><style>body{font-family:Arial;margin:20px;text-align:center;}</style></head>
<body>
  <h2>[OK] Configuration Saved Successfully!</h2>
  <p>Changes applied immediately. Light controller updated.</p>
  <p><a href="/">Back to Config</a></p>
</body>
</html>
)rawliteral";

const char HTML_RESET[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head><meta charset="UTF-8"><title>Reset</title><style>body{font-family:Arial;margin:20px;text-align:center;}</style></head>
<body>
  <h2>[RST] Configuration Reset to Defaults!</h2>
  <p>All settings restored and applied immediately.</p>
  <p><a href="/">Back to Config</a></p>
</body>
</html>
)rawliteral";

const char HTML_DEBUG[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Debug Info</title>
  <style>
    body { font-family: monospace; margin: 20px; background: #f5f5f5; }
    .container { max-width: 600px; margin: 0 auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
    h2 { color: #333; }
    .debug-item { margin: 10px 0; padding: 10px; background: #f9f9f9; border-left: 3px solid #2196F3; }
    .label { font-weight: bold; color: #555; }
    .value { color: #2196F3; margin-left: 10px; }
    hr { margin: 20px 0; border: none; border-top: 2px solid #eee; }
    button { padding: 10px 20px; margin-top: 20px; background-color: #4CAF50; color: white; border: none; border-radius: 4px; cursor: pointer; font-weight: bold; font-size: 14px; }
    button:hover { background-color: #45a049; }
  </style>
</head>
<body>
  <div class="container">
    <h2>[DBG] System Debug Information</h2>
    <div class="debug-item">
      <span class="label">Uptime (ms):</span>
      <span class="value">%UPTIME%</span>
    </div>
    <div class="debug-item">
      <span class="label">Free Heap (bytes):</span>
      <span class="value">%HEAP%</span>
    </div>
    <div class="debug-item">
      <span class="label">Roll Raw (deg):</span>
      <span class="value">%ROLL_RAW%</span>
    </div>
    <div class="debug-item">
      <span class="label">Roll Filtered (deg):</span>
      <span class="value">%ROLL_FILTERED%</span>
    </div>
    <hr>
    <h3 style="color: #666;">Accelerometer Raw Values (m/s2)</h3>
    <div class="debug-item">
      <span class="label">Accel X:</span>
      <span class="value">%ACCEL_X%</span>
    </div>
    <div class="debug-item">
      <span class="label">Accel Y:</span>
      <span class="value">%ACCEL_Y%</span>
    </div>
    <div class="debug-item">
      <span class="label">Accel Z:</span>
      <span class="value">%ACCEL_Z%</span>
    </div>
    <div class="debug-item">
      <span class="label">PWM Left:</span>
      <span class="value">%PWM_LEFT%</span>
    </div>
    <div class="debug-item">
      <span class="label">PWM Right:</span>
      <span class="value">%PWM_RIGHT%</span>
    </div>
    <div class="debug-item">
      <span class="label">Clients Connected:</span>
      <span class="value">%CLIENTS%</span>
    </div>
    <hr>
    <h3 style="color: #666;">MPU6050 Calibration Offsets</h3>
    <div class="debug-item">
      <span class="label">Accel Offset X:</span>
      <span class="value">%OFFSET_X%</span>
    </div>
    <div class="debug-item">
      <span class="label">Accel Offset Y:</span>
      <span class="value">%OFFSET_Y%</span>
    </div>
    <div class="debug-item">
      <span class="label">Accel Offset Z:</span>
      <span class="value">%OFFSET_Z%</span>
    </div>
    <div class="debug-item">
      <span class="label">Debug Mode:</span>
      <span class="value" style="color: %DEBUG_COLOR%;">%DEBUG_MODE%</span>
    </div>
    <hr>
    <button onclick="location.href='/'">Back to Configuration</button>
  </div>
</body>
</html>
)rawliteral";

APConfigServer::APConfigServer() 
  : server(80), bootTime(0), lastClientDisconnectTime(0), configWindowOpen(true), clientConnected(false), configSaved(false) {}

void APConfigServer::handleRoot() {
  String html = String(HTML_ROOT);
  // Cargar valores actuales en los inputs usando placeholders
  html.replace("%ANGLE_ON%", String(cfg.angleOn, 1));
  html.replace("%ANGLE_OFF%", String(cfg.angleOff, 1));
  html.replace("%MAX_PWM%", String(cfg.maxPWM));
  html.replace("%FILTER_ALPHA%", String(cfg.filterAlpha, 2));
  html.replace("%FADE_STEP%", String(cfg.fadeStep));
  html.replace("%OFFSET_X%", String(cfg.accelOffsetX, 3));
  html.replace("%OFFSET_Y%", String(cfg.accelOffsetY, 3));
  html.replace("%OFFSET_Z%", String(cfg.accelOffsetZ, 3));
  html.replace("%DEBUG_CHECKED%", cfg.debugMode ? "checked" : "");
  
  server.send(200, "text/html", html);
}

void APConfigServer::handleSave() {
  cfg.angleOn = server.arg("aon").toFloat();
  cfg.angleOff = server.arg("aoff").toFloat();
  cfg.maxPWM = server.arg("pwm").toInt();
  cfg.filterAlpha = server.arg("alpha").toFloat();
  cfg.fadeStep = server.arg("fade").toInt();
  
  // Cargar offsets de calibración si están en los argumentos
  if (server.hasArg("ox")) {
    cfg.accelOffsetX = server.arg("ox").toFloat();
  }
  if (server.hasArg("oy")) {
    cfg.accelOffsetY = server.arg("oy").toFloat();
  }
  if (server.hasArg("oz")) {
    cfg.accelOffsetZ = server.arg("oz").toFloat();
  }
  
  // Cargar modo debug
  cfg.debugMode = server.hasArg("debug");
  
  // Validar valores
  cfg.angleOn = constrain(cfg.angleOn, 0, 90);
  cfg.angleOff = constrain(cfg.angleOff, 0, 90);
  cfg.maxPWM = constrain(cfg.maxPWM, 0, 1023);
  cfg.filterAlpha = constrain(cfg.filterAlpha, 0, 1);
  cfg.fadeStep = constrain(cfg.fadeStep, 1, 50);
  
  // Validar que angleOff < angleOn
  if (cfg.angleOff >= cfg.angleOn) {
    cfg.angleOff = cfg.angleOn - 1.0;
  }
  
  saveConfig();
  loadConfig();  // Hot-reload: apply changes immediately
  
  String response = String(HTML_SAVED);
  server.send(200, "text/html", response);
  
  // Keep WiFi running - don't reboot or close connection
  Serial.println("Configuration saved and applied immediately!");
}

void APConfigServer::handleStatus() {
  String json = "{";
  json += "\"angleOn\":" + String(cfg.angleOn, 2) + ",";
  json += "\"angleOff\":" + String(cfg.angleOff, 2) + ",";
  json += "\"maxPWM\":" + String(cfg.maxPWM) + ",";
  json += "\"filterAlpha\":" + String(cfg.filterAlpha, 4) + ",";
  json += "\"fadeStep\":" + String(cfg.fadeStep);
  json += "}";
  server.send(200, "application/json", json);
}

void APConfigServer::handleReset() {
  resetConfig();
  loadConfig();  // Hot-reload: apply default values immediately
  
  String response = String(HTML_RESET);
  server.send(200, "text/html", response);
  
  // Keep WiFi running - don't reboot or close connection
  Serial.println("Configuration reset and applied immediately!");
}

void APConfigServer::handleDebug() {
  extern MPUHandler mpu;
  extern LightController lights;
  
  String html = String(HTML_DEBUG);
  
  // Reemplazar valores en la plantilla
  html.replace("%UPTIME%", String(millis()));
  html.replace("%HEAP%", String(ESP.getFreeHeap()));
  html.replace("%ROLL_RAW%", String(mpu.getRawRoll(), 2));
  html.replace("%ROLL_FILTERED%", String(mpu.getRoll(), 2));
  html.replace("%ACCEL_X%", String(mpu.getAccelX(), 2));
  html.replace("%ACCEL_Y%", String(mpu.getAccelY(), 2));
  html.replace("%ACCEL_Z%", String(mpu.getAccelZ(), 2));
  html.replace("%PWM_LEFT%", String(lights.getLeftPWM()));
  html.replace("%PWM_RIGHT%", String(lights.getRightPWM()));
  html.replace("%CLIENTS%", String(WiFi.softAPgetStationNum()));
  html.replace("%OFFSET_X%", String(cfg.accelOffsetX, 3));
  html.replace("%OFFSET_Y%", String(cfg.accelOffsetY, 3));
  html.replace("%OFFSET_Z%", String(cfg.accelOffsetZ, 3));
  html.replace("%DEBUG_MODE%", cfg.debugMode ? "ON (Simulated)" : "OFF (Real Sensor)");
  html.replace("%DEBUG_COLOR%", cfg.debugMode ? "#FFA500" : "#00AA00");
  
  server.send(200, "text/html", html);
}

void APConfigServer::handleCalibrate() {
  if (server.hasArg("ax") && server.hasArg("ay") && server.hasArg("az")) {
    extern MPUHandler mpu;
    float ax = server.arg("ax").toFloat();
    float ay = server.arg("ay").toFloat();
    float az = server.arg("az").toFloat();
    mpu.calibrateAccel(ax, ay, az);
    saveConfig();
    server.send(200, "application/json", "{\"status\":\"calibrated\"}");
  } else {
    server.send(400, "application/json", "{\"error\":\"Missing parameters\"}");
  }
}

void APConfigServer::begin() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
  
  IPAddress apIP = WiFi.softAPIP();
  Serial.print("WiFi AP started. Connect to: ");
  Serial.print(WIFI_SSID);
  Serial.print(" IP: ");
  Serial.println(apIP);
  
  server.on("/", std::bind(&APConfigServer::handleRoot, this));
  server.on("/save", std::bind(&APConfigServer::handleSave, this));
  server.on("/status", std::bind(&APConfigServer::handleStatus, this));
  server.on("/reset", std::bind(&APConfigServer::handleReset, this));
  server.on("/debug", std::bind(&APConfigServer::handleDebug, this));
  server.on("/calibrate", std::bind(&APConfigServer::handleCalibrate, this));
  
  server.begin();
  bootTime = millis();
}

void APConfigServer::handleClient() {
  server.handleClient();
  
  // Actualizar estado de conexión
  int stationNum = WiFi.softAPgetStationNum();
  if (stationNum > 0) {
    if (!clientConnected) {
      clientConnected = true;
      Serial.println("[WiFi] Client connected - timeout disabled");
    }
  } else {
    if (clientConnected) {
      lastClientDisconnectTime = millis();
      clientConnected = false;
      Serial.println("[WiFi] Client disconnected - 30s timeout started");
    }
  }
}

bool APConfigServer::shouldCloseConfigWindow() const {
  // No cerrar automáticamente después de guardar - permitir más ajustes
  // La ventana de configuración permanece abierta hasta que no hay clientes por 30 segundos
  
  // Si hay cliente conectado, no cerrar (infinito)
  if (clientConnected) {
    return false;
  }
  
  // Si no hay cliente, cerrar después de 30 segundos
  if (millis() - lastClientDisconnectTime >= WIFI_TIMEOUT) {
    return true;
  }
  
  return false;
}

void APConfigServer::closeConfigWindow() {
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  configWindowOpen = false;
  Serial.println("Config window closed. Entering normal mode.");
}
