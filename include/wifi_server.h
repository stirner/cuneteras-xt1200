#ifndef WIFI_SERVER_H
#define WIFI_SERVER_H

#include <ESP8266WebServer.h>

class APConfigServer {
private:
  ESP8266WebServer server;
  unsigned long bootTime;
  unsigned long lastClientDisconnectTime;
  bool configWindowOpen;
  bool clientConnected;
  bool configSaved;
  
  void handleRoot();
  void handleSave();
  void handleStatus();
  void handleReset();
  void handleDebug();
  void handleCalibrate();
  
public:
  APConfigServer();
  
  void begin();
  void handleClient();
  bool isConfigWindowOpen() const { return configWindowOpen; }
  bool shouldCloseConfigWindow() const;
  void closeConfigWindow() __attribute__((unused));
  bool hasClientConnected() const { return clientConnected; }
  bool isConfigSaved() const { return configSaved; }
};

#endif
