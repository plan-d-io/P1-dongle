#include "rom/rtc.h"
#include <esp_int_wdt.h>
#include <esp_task_wdt.h>
#include "M5Atom.h"
#include <LittleFS.h>
#define SPIFFS LittleFS
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <Preferences.h>
#include <PubSubClient.h>
#include <time.h>
#include <Update.h>
#include "ArduinoJson.h"
#include <elapsedMillis.h>
#include "UUID.h"
#include "configStore.h"
#include "ledControl.h"
#include "externalIntegrations.h"
#include "WebRequestHandler.h"
#include "webHelp.h"
#define HWSERIAL Serial1
#define TRIGGER 25 //Pin to trigger meter telegram request

unsigned int fw_ver = 223;

//General global vars
Preferences preferences;
AsyncWebServer server(80);
DNSServer dnsServer;
uint8_t* certData = nullptr; 
WiFiClient wificlient;
PubSubClient mqttclient(wificlient);
WiFiClientSecure *client = new WiFiClientSecure;
PubSubClient mqttclientSecure(*client);
HTTPClient https;
UUID uuid;
bool clientSecureBusy, mqttPaused, mqttWasPaused, resetWifi, factoryReset, updateAvailable;
bool wifiError, mqttWasConnected, wifiSave, wifiScan, debugInfo, timeconfigured, timeSet, spiffsMounted,rebootInit;
bool bundleLoaded = true;
bool haDiscovered = false;
String configBuffer, resetReason, infoMsg, ssidList;
char apSSID[] = "P1000000";
unsigned int mqttPushCount, mqttPushFails, onlineVersion, fw_new;
unsigned int secureClientError = 0;
time_t meterTimestamp;
//Global timing vars
elapsedMillis sinceConnCheck, sinceUpdateCheck, sinceClockCheck, sinceLastUpload, sinceDebugUpload, sinceRebootCheck, sinceMeterCheck, sinceWifiCheck, sinceTelegramRequest;
//General housekeeping vars
unsigned int reconncount, remotehostcount, telegramCount, telegramAction;
int wifiRSSI;
float freeHeap, minFreeHeap, maxAllocHeap;
byte mac[6];
uint8_t prevButtonState = false;
/*Debug*/
bool serialDebug = true;
bool telegramDebug = false;
bool mqttDebug = false;
bool httpDebug = false;
bool extendedTelegramDebug = false;

void setup(){
  M5.begin(true, false, true);
  delay(2000);
  pinMode(TRIGGER, OUTPUT);
  setBuff(0x00, 0xff, 0x00); //red 
  M5.dis.displaybuff(DisBuff);
  unitState = -1;
  Serial.begin(115200);
  HWSERIAL.begin(115200, SERIAL_8N1, 21, 22);
  delay(500);
  getHostname();
  Serial.println();
  syslog("Digital meter dongle booting", 0);
  restoreConfig();
  initSPIFFS();
  externalIntegrationsBootstrap();
  if(_trigger_type == 0) digitalWrite(TRIGGER, HIGH);
  else digitalWrite(TRIGGER, LOW);
  syslog("Digital meter dongle " + String(apSSID) +" V" + String(fw_ver/100.0) + " by plan-d.io", 1);
  if(_dev_fleet) syslog("Using experimental (development) firmware", 2); //change this to one variable, but keep legacy compatibility intact
  if(_alpha_fleet) syslog("Using pre-release (alpha) firmware", 0);
  if(_v2_fleet) syslog("Using V2.0 firmware", 0);
  syslog("Checking if internal clock is set", 0);
  printLocalTime(true);
  _bootcount = _bootcount + 1;
  syslog("Boot #" + String(_bootcount), 1);
  saveBoots();
  get_reset_reason(rtc_get_reset_reason(0));
  syslog("Last reset reason (hardware): " + resetReason, 1);
  syslog("Last reset reason (firmware): " + _last_reset, 1);
  debugInfo = true;
  initWifi();
  server.addHandler(new WebRequestHandler());
  server.begin();
  configBuffer = returnConfig();
  String availabilityTopic = _mqtt_prefix.substring(0, _mqtt_prefix.length()-1);
  Serial.println("Done");
}

void loop(){
  blinkLed();
  if(wifiScan) scanWifi();
  if(sinceRebootCheck > 2000){
    if(rebootInit){
      forcedReset();
    }
    sinceRebootCheck = 0;
  }
  if(_trigger_type == 0){
    /*Continous triggering of meter telegrams*/
    if(sinceMeterCheck > 30000){
      if(!meterError) syslog("Meter disconnected", 2);
      meterError = true;
      if(_wifi_STA && unitState < 7) unitState = 6;
      else if(!_wifi_STA && unitState < 3) unitState = 2;
      sinceMeterCheck = 0;
    }
  }
  else if(_trigger_type == 1){
    /*On demand triggering of meter telegram*/
    if(sinceTelegramRequest >= _trigger_interval *1000){
      digitalWrite(TRIGGER, HIGH);
      sinceTelegramRequest = 0;
    }
    if(sinceMeterCheck > (_trigger_interval *1000) + 30000){
      syslog("Meter disconnected", 2);
      meterError = true;
      if(_wifi_STA && unitState < 7) unitState = 6;
      else if(!_wifi_STA && unitState < 3) unitState = 2;
      sinceMeterCheck = 0;
    }
  }
  if(!_wifi_STA){
    /*If dongle is in access point mode*/
    dnsServer.processNextRequest();
    if(sinceWifiCheck >= 600000){
      /*If dongle is in AP mode, check every once in a while if the configured wifi SSID can't be detected
       * If so, reboot so the dongle starts up again in connected STA mode. */
      if(scanWifi()){
        saveResetReason("Found saved WiFi SSID, rebooting to reconnect");
        if(saveConfig()){
          syslog("Found saved WiFi SSID, rebooting to reconnect", 1);
          setReboot();
        }
      }
      sinceWifiCheck = 0;
    }
    if(sinceClockCheck >= 600000){
      timeSet = false;
      sinceClockCheck = 0;
    }
  }
  else{
    /*If dongle is connected to wifi*/
    if(!bundleLoaded) restoreSPIFFS();
    if(_mqtt_en){
      if(_mqtt_tls){
        mqttclientSecure.loop();
      }
      else{
        mqttclient.loop();
      }
    }
    if(lastEIDcheck >= EIDcheckInterval){
      eidHello();
    }
    if(lastEIDupload > EIDuploadInterval){
      eidUpload();
    }
    if(_update_autoCheck && sinceUpdateCheck >= 86400000){
      updateAvailable = checkUpdate();
      if(updateAvailable) startUpdate();
      sinceUpdateCheck = 0;
    }
    if(sinceClockCheck >= 3600){
      if(!timeconfigured) timeSet = false; //if timeConfigured = true, the NTP serivce takes care of reqular clock syncing
      sinceClockCheck = 0;
    }
    if(sinceConnCheck >= 60000){
      if(_ha_en && debugInfo) hadebugDevice(false);
      checkConnection();
      sinceConnCheck = 0;
    }
    if(sinceDebugUpload >= 300000){
      getHeapDebug();
      sinceDebugUpload = 0;
    }
    /*If no remote hosts can be reached anymore, try a reboot for up to four times*/
    if(reconncount > 15 || remotehostcount > 60 || secureClientError > 4){
      _rebootSecure++;
      if(_rebootSecure < 4){
        saveResetReason("Rebooting to try fix connections");
        if(saveConfig()){
          syslog("Rebooting to try fix connections", 2);
          setReboot();
        }
        reconncount = 0;
      }
      else{
        /*After four reboots, increase time between reboots drastically*/
        if(reconncount > 150 || remotehostcount > 600 || secureClientError > 40){
          _rebootSecure++;
          saveResetReason("Rebooting to try fix connections");
          if(saveConfig()){
            syslog("Rebooting to try fix connections", 2);
            setReboot();
          }
          reconncount = 0;
        }
      }
    }
  }
  M5.update();
  if (M5.Btn.pressedFor(2000)) {
    resetWifi = true;
  }
  if (M5.Btn.pressedFor(5000)) {
    resetWifi = false;
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
  if(HWSERIAL.available() > 0) {
    /*Read the received meter telegram. A telegram ends on the '!' character, followed by a 4-digit CRC16 value*/
    String telegram=HWSERIAL.readStringUntil('!');
    telegram = telegram + '\n';
    String crc =  HWSERIAL.readStringUntil('\n');
    if(_trigger_type == 1){
      digitalWrite(TRIGGER, LOW);
      sinceTelegramRequest = 0;
    }
    processMeterTelegram(telegram, crc);
  }
}
