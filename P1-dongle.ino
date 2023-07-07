#include "rom/rtc.h"
#include "M5Atom.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include <Preferences.h>
#include <PubSubClient.h>
#include "time.h"
#include "FS.h"
#include <Update.h>
#include "ArduinoJson.h"
#include <elapsedMillis.h>

/*V2.0 declarations*/
#include "externalIntegrations.h"
#include "WebRequestHandler.h" //
#include "configStore.h"
#include "webHelp.h"

boolean resetWifi, factoryReset; 
Preferences preferences; //
AsyncWebServer server(80); //
#define HWSERIAL Serial1 //

unsigned long keyPushList = 65534;//511;
unsigned int mbusPushList = 136;
int payloadFormat = 3; //0 = value only, 1 = minimal json, 2 = standard json, 3 = COFY format

bool serialDebug = true;
bool telegramDebug = true;
bool mqttDebug = false;
bool extendedTelegramDebug;
bool realto = true;
bool haMakeAutoDiscovery = true;
bool ha_metercreated;
bool pushFullTelegram;
unsigned int fw_ver = 105;
int telegramCount;
String mqttPrefix = "data/devices/utility_meter/"; //temp, needs to be loaded from preferences!
time_t meterTimestamp;

float totConT1, totConT2, totCon, totInT1, totInT2, totIn, powCon, powIn, netPowCon, totGasCon, volt1, volt2, volt3, avgDem, maxDemM;
String mbusTempKey = "0-1:24.2.1";

struct mbusMeterType {
      String mbusKey;
      int type = 0; //3 = gas meter, 4 = heat/cold, 7 = water meter
      int measurementType = 0; //1 = base value, 3 = non-temperature compensated
      String id;
      bool enabled;
};
mbusMeterType mbusMeter[4];

struct keyConfig {
  String dsmrKey;
  uint8_t keyType;
  String deviceType;
  String  keyName;
  String  keyTopic;
  bool retain;
};

struct mbusConfig {
  uint8_t keyType; //3 = gas meter, 4 = heat/cold, 7 = water meter
  String deviceType;
  String  keyName;
  String  keyTopic;
  bool retain;
};
/*Legacy declarations*/
unsigned int onlineVersion, fw_new;
struct tm dm_time;  // dm time elements structure
time_t dm_timestamp; // dm timestamp
DNSServer dnsServer;
WiFiClient wificlient;
PubSubClient mqttclient(wificlient);
WiFiClientSecure *client = new WiFiClientSecure;
PubSubClient mqttclientSecure(*client);
HTTPClient https;
bool bundleLoaded = true;
bool clientSecureBusy, mqttPaused;
bool updateAvailable;

#define TRIGGER 25 //Pin to trigger meter telegram request
//Global timing vars
elapsedMillis sinceConnCheck, sinceUpdateCheck, sinceClockCheck, sinceLastUpload, sinceRebootCheck, sinceMeterCheck, sinceWifiCheck, sinceTelegramRequest;
//LED state machine vars
uint8_t DisBuff[2 + 5 * 5 * 3];
elapsedMillis ledTime;
boolean ledState = true;
byte unitState = 0;

//General housekeeping vars
unsigned int reconncount, remotehostcount;
String resetReason, last_reset_verbose;
float freeHeap, minFreeHeap, maxAllocHeap;
String ssidList;
char apSSID[] = "COFY0000";
byte mac[6];
boolean rebootReq = false;
boolean rebootInit = false;
boolean wifiError, mqttHostError, mqttClientError, mqttWasConnected, httpsError, meterError, eidError, wifiSave, wifiScan, eidSave, mqttSave, haSave, debugInfo, timeconfigured;


boolean timeSet, mTimeFound, spiffsMounted;

uint8_t prevButtonState = false;




void setup(){
  M5.begin(true, false, true);
  delay(2000);
  pinMode(TRIGGER, OUTPUT);
  setBuff(0x00, 0xff, 0x00); //red 
  M5.dis.displaybuff(DisBuff);
  Serial.begin(115200);
  HWSERIAL.begin(115200, SERIAL_8N1, 21, 22);
  delay(500);
  getHostname();
  Serial.println();
  syslog("Digital meter dongle booting", 0);
  restoreConfig();
  initSPIFFS();
  if(_dev_fleet) syslog("Using experimental (development) firmware", 2); //change this to one variable, but keep legacy compatibility intact
  if(_alpha_fleet) syslog("Using pre-release (alpha) firmware", 0);
  syslog("Checking if internal clock is set", 0);
  printLocalTime(true);
  _bootcount = _bootcount + 1;
  syslog("Boot #" + String(_bootcount), 1);
  saveBoots();
  get_reset_reason(rtc_get_reset_reason(0));
  syslog("Last reset reason: " + resetReason, 1);
  syslog("Last reset reason (verbose): " + last_reset_verbose, 1);
  debugInfo = true;
  initWifi();
  scanWifi();
  server.addHandler(new WebRequestHandler());
  server.begin();
}

void loop(){
  blinkLed();
  if(wifiScan) scanWifi();
  if(sinceRebootCheck > 2000){
    if(rebootInit){
      if(!clientSecureBusy){
        ESP.restart();
      } 
    }
    sinceRebootCheck = 0;
  }
  if(sinceMeterCheck > 90000){
    syslog("Meter disconnected", 2);
    meterError = true;
    sinceMeterCheck = 0;
  }
  if(_trigger_type == 1){
    if(sinceTelegramRequest >= _trigger_interval *1000){
      digitalWrite(TRIGGER, HIGH);
      sinceTelegramRequest = 0;
    }
  }
  if(!_wifi_STA){
    dnsServer.processNextRequest();
    if(!timeSet) setMeterTime();
    if(sinceWifiCheck >= 600000){
      if(scanWifi()) rebootInit = true;
      sinceWifiCheck = 0;
    }
    if(sinceClockCheck >= 3600){
      setMeterTime();
      sinceClockCheck = 0;
    }
    if(mTimeFound && ! meterError) unitState = 2;
    else unitState = 3;
  }
  else{
    if(!bundleLoaded) restoreSPIFFS();
    if(_mqtt_tls){
      mqttclientSecure.loop();
    }
    else{
      mqttclient.loop();
    }
    if(_update_autoCheck && sinceUpdateCheck >= 86400000){
      updateAvailable = checkUpdate();
      if(updateAvailable) startUpdate();
      sinceUpdateCheck = 0;
    }
    if(sinceClockCheck >= 3600){
      if(timeconfigured) setMeterTime();
      sinceClockCheck = 0;
    }
    if(sinceConnCheck >= 60000){
      checkConnection();
      getHeapDebug();
      sinceConnCheck = 0;
    }
    if(wifiError || mqttHostError || mqttClientError || httpsError || meterError || !spiffsMounted) unitState = 5;
    else unitState = 4;
    if(reconncount > 15 || remotehostcount > 60){
      last_reset = "Rebooting to try fix connections";
      if(saveConfig()){
        syslog("Rebooting to try fix connections", 2);
        setReboot();
      }
      reconncount = 0;
    }
  }
  M5.update();
  if (M5.Btn.pressedFor(2000)) {
    resetWifi = true;
  }
  if (M5.Btn.pressedFor(5000)) {
    factoryReset = true;
  }
  if (prevButtonState != M5.Btn.isPressed()) {
    if(!M5.Btn.isPressed()){
      if(factoryReset || resetWifi){
        resetConfig();
      }
    }
  }
  prevButtonState = M5.Btn.isPressed();
  if (HWSERIAL.available() > 0) {
    String s=HWSERIAL.readStringUntil('!');
    s = s + HWSERIAL.readStringUntil('\n');
    s = s + '\n';
    if(_trigger_type == 1){
      digitalWrite(TRIGGER, LOW);
      sinceTelegramRequest = 0;
    }
    if(meterError){
      meterError = false;
    }
    processMeterTelegram(s);
  }
}
