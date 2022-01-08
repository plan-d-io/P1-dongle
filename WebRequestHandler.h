#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <WiFi.h>
#include <Preferences.h>

extern void scanWifi(void), haAutoDiscovery(boolean);
extern boolean saveConfig(void), startUpdate(void);
extern String getHostname(void), getConfig(void), getIndexData(void), getIndexStatic(void);

class WebRequestHandler : public AsyncWebHandler {
public:
  WebRequestHandler() {}
  virtual ~WebRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request);

  void handleRequest(AsyncWebServerRequest *request);

private:
  String _hostname;
};
