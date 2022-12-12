#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <WiFi.h>
#include <Preferences.h>

extern void haAutoDiscovery(boolean), syslog(String, int), setReboot(void);
extern boolean saveConfig(void), startUpdate(void), scanWifi(void);
extern String getHostname(void), getConfig(void), getEmail(void), getIndexData(void), getIndexStatic(void), getUnit(void), getDm(void), getIo(void);

class WebRequestHandler : public AsyncWebHandler {
public:
  WebRequestHandler() {}
  virtual ~WebRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request);

  void handleRequest(AsyncWebServerRequest *request);

private:
  String _hostname;
};
