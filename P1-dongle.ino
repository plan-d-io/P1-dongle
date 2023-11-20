<<<<<<< HEAD
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
  #include "WebRequestHandler.h"
  #include <PubSubClient.h>
  #include "time.h"
  #include "FS.h"
  #include <Update.h>
  #include "ArduinoJson.h"
  #include <elapsedMillis.h>
  #include "ledControl.h"
  
  unsigned int fw_ver = 108;
  unsigned int onlineVersion, fw_new;
  DNSServer dnsServer;
  AsyncWebServer server(80);
  WiFiClient wificlient;
  PubSubClient mqttclient(wificlient);
  WiFiClientSecure *client = new WiFiClientSecure;
  PubSubClient mqttclientSecure(*client);
  HTTPClient https;
  bool bundleLoaded = true;
  bool clientSecureBusy, mqttPaused;
  
  #define HWSERIAL Serial1 //Use hardware UART for communication with digital meter
  #define TRIGGER 25 //Pin to trigger meter telegram request
  
  //Data structure for pulse counters
  struct pulse {
      uint32_t numberPulses;
      bool validPulse;
  };
  pulse pulse1 = {0, false};
  pulse pulse2 = {0, false};
  //variables to keep track of the timing of recent interrupts
  unsigned long pulse_time1, pulse_time2;  
  unsigned long last_pulse_time1, last_pulse_time2; 
  unsigned int pls_off1, pls_off2;
  
  void IRAM_ATTR pulseCounter1() {
    int input_state = digitalRead(32);
    if(input_state == 0){
      pulse_time1 = millis();
    }
    else{
      if (pulse_time1 - last_pulse_time1 > 1500){
        pulse1.numberPulses++;
        pulse1.validPulse = true;
        last_pulse_time1 = pulse_time1;
      }
    }
  }
  
  void IRAM_ATTR pulseCounter2() {
    int input_state = digitalRead(26);
    if(input_state == 0){
      pulse_time2 = millis();
    }
    else{
      if (pulse_time2 - last_pulse_time2 > 1500){
        pulse2.numberPulses++;
        pulse2.validPulse = true;
        last_pulse_time2 = pulse_time2;
      }
    }
  }
  
  //Global timing vars
  elapsedMillis sinceConnCheck, sinceUpdateCheck, sinceClockCheck, sinceLastUpload, sinceEidUpload, sinceLastWebRequest, sinceRebootCheck, sinceMeterCheck, sinceWifiCheck, sinceTelegramRequest;
  
  //Global vars to store basic digital meter telegram values
  float totConDay, totConNight, totCon, totInDay, totInNight, totIn, totPowCon, totPowIn, netPowCon, totGasCon, volt1, volt2, volt3, avgDem, maxDemM;
  String jsonData;
  RTC_NOINIT_ATTR float totConToday, totConYesterday, gasConToday, gasConYesterday;
  //Pulse input vars
  boolean pls_en, pls_emu;
  int pls_mind1, pls_mind2, pls_multi1, pls_multi2, pls_type1, pls_type2, pls_emuchan;
  String pls_unit1, pls_unit2;
  //Meter telegram timestamp vars
  struct tm dm_time;  // dm time elements structure
  time_t dm_timestamp; // dm timestamp
  struct tm mb1_time;  // mbus1 time elements structure
  time_t mb1_timestamp; // mbus1 timestamp
  int prevDay = -1;
  
  //General housekeeping vars
  unsigned int counter, bootcount, refbootcount, reconncount, remotehostcount;
  String resetReason, last_reset, last_reset_verbose;
  float freeHeap, minFreeHeap, maxAllocHeap;
  Preferences preferences;  
  String ssidList;
  char apSSID[] = "COFY0000";
  byte mac[6];
  boolean wifiSTA = false;
  boolean rebootReq = false;
  boolean rebootInit = false;
  boolean wifiError, mqttHostError, mqttClientError, mqttWasConnected, httpsError, meterError, eidError, wifiSave, wifiScan, eidSave, mqttSave, haSave, debugInfo, timeconfigured, firstDebugPush, alpha_fleet, dev_fleet;
  String dmPowIn, dmPowCon, dmTotCont1, dmTotCont2, dmTotInt1, dmTotInt2, dmActiveTariff, dmVoltagel1, dmVoltagel2, dmVoltagel3, dmCurrentl1, dmCurrentl2, dmCurrentl3, dmGas, dmText, dmAvDem, dmMaxDemM;
  String meterConfig[17];
  int dsmrVersion, trigger_type, trigger_interval;
  boolean timeSet, mTimeFound, spiffsMounted;
  String wifi_ssid, wifi_password;
  String mqtt_host, mqtt_id, mqtt_user, mqtt_pass;
  uint8_t prevButtonState = false;
  boolean configSaved, resetWifi, resetAll;
  boolean mqtt_en, mqtt_tls, mqtt_auth;
  boolean update_autoCheck, update_auto, updateAvailable, update_start, update_finish, restore_finish, eid_en, ha_en, ha_metercreated, reinit_spiffs;
  unsigned int mqtt_port;
  unsigned long upload_throttle;
  String eid_webhook;
  
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
    preferences.begin("cofy-config", true);
    delay(100);
    initConfig();
    delay(100);
    restoreConfig();
    initSPIFFS();
    syslog("Digital meter dongle " + String(apSSID) +" V" + String(fw_ver/100.0) + " by plan-d.io", 1);
    if(dev_fleet) syslog("Using experimental (development) firmware", 2);
    if(alpha_fleet) syslog("Using pre-release (alpha) firmware", 0);
    syslog("Checking if internal clock is set", 0);
    printLocalTime(true);
    bootcount = bootcount + 1;
    syslog("Boot #" + String(bootcount), 1);
    saveBoots();
    get_reset_reason(rtc_get_reset_reason(0));
    syslog("Last reset reason: " + resetReason, 1);
    syslog("Last reset reason (verbose): " + last_reset_verbose, 1);
    debugInfo = true;
    if(trigger_type == 0) digitalWrite(TRIGGER, HIGH);
    else digitalWrite(TRIGGER, LOW);
    if(pls_en){
      pinMode(32, INPUT_PULLUP);
      pinMode(26, INPUT_PULLUP);
      attachInterrupt(32, pulseCounter1, CHANGE);
      attachInterrupt(26, pulseCounter2, CHANGE);
    }
    delay(100);
    initWifi();
    scanWifi();
    server.begin();
  }
  
  void loop(){

    blinkLed();
    if(mqtt_tls){
      mqttclientSecure.loop();
    }
    else{
      mqttclient.loop();
    }
    if(wifiScan) scanWifi();
    if(sinceRebootCheck > 2000){
      if(rebootInit){
        if(!clientSecureBusy){
          ESP.restart();
        }
        
      }
      sinceRebootCheck = 0;
    }
    if(trigger_type == 0){
      if(sinceMeterCheck > 30000){
        if(!meterError) syslog("Meter disconnected", 2);
        meterError = true;
        if(wifiSTA && unitState < 7) unitState = 6;
        else if(!wifiSTA && unitState < 3) unitState = 2;
        sinceMeterCheck = 0;
      }
    }
    else if(trigger_type == 1){
      if(sinceTelegramRequest >= trigger_interval *1000){
        digitalWrite(TRIGGER, HIGH);
        sinceTelegramRequest = 0;
      }
      if(sinceMeterCheck > (trigger_interval *1000) + 30000){
        syslog("Meter disconnected", 2);
        meterError = true;
        if(wifiSTA && unitState < 7) unitState = 6;
        else if(!wifiSTA && unitState < 3) unitState = 2;
        sinceMeterCheck = 0;
      }
    }
    if(!wifiSTA){
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
      if(mTimeFound && ! meterError && !wifiError) unitState = 0;
      else if(meterError) unitState = 2;
      else if(wifiError) unitState = 1;
    }
    else{
      if(!bundleLoaded) restoreSPIFFS();
      if(update_autoCheck && sinceUpdateCheck >= 86400000){
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
      if(eid_en && mTimeFound && sinceEidUpload > 15*60*1000){
        if(eidUpload()) sinceEidUpload = 0;
        else sinceEidUpload = (15*60*900000)-(5*60*1000);
      }
      if(meterError) unitState = 6;
      else if(wifiError) unitState = 1;
      else if(mqttHostError || mqttClientError || httpsError) unitState = 5;
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
      resetAll = true;
    }
    if (prevButtonState != M5.Btn.isPressed()) {
      if(!M5.Btn.isPressed()){
        if(resetAll || resetWifi){
          resetConfig();
        }
=======
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
#include "configStore.h"
#include "ledControl.h"
#include "externalIntegrations.h"
#include "WebRequestHandler.h"
#include "webHelp.h"
#define HWSERIAL Serial1
#define TRIGGER 25 //Pin to trigger meter telegram request

unsigned int fw_ver = 209;

//General global vars
Preferences preferences;
AsyncWebServer server(80);
DNSServer dnsServer;
WiFiClient wificlient;
PubSubClient mqttclient(wificlient);
WiFiClientSecure *client = new WiFiClientSecure;
PubSubClient mqttclientSecure(*client);
HTTPClient https;
bool bundleLoaded = true;
bool clientSecureBusy, mqttPaused, resetWifi, factoryReset, updateAvailable;
String configBuffer;
String eidUploadInterval = "Not yet set";
unsigned int mqttPushCount, mqttPushFails, onlineVersion, fw_new;
bool wifiError, mqttWasConnected, wifiSave, wifiScan, debugInfo, timeconfigured, timeSet, spiffsMounted;
bool haDiscovered = false;
time_t meterTimestamp;
uint8_t prevButtonState = false;
//Global timing vars
elapsedMillis sinceConnCheck, sinceUpdateCheck, sinceClockCheck, sinceLastUpload, sinceDebugUpload, sinceRebootCheck, sinceMeterCheck, sinceWifiCheck, sinceTelegramRequest;
//General housekeeping vars
unsigned int reconncount, remotehostcount, telegramCount;
int wifiRSSI;
String resetReason, infoMsg;
float freeHeap, minFreeHeap, maxAllocHeap;
String ssidList;
char apSSID[] = "P1000000";
byte mac[6];
bool rebootInit;
/*Debug*/
bool serialDebug = true;
bool telegramDebug = false;
bool mqttDebug = false;
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
  //if(_wifi_STA) eidHello();
  Serial.println("Done");
  
}

void loop(){
  blinkLed();
  if(wifiScan) scanWifi();

  if(sinceRebootCheck > 2000){
    if(rebootInit){
      //ESP.restart();
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
      if(scanWifi()) rebootInit = true;
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
      //if(_realto_en) realtoUpload();
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
    }/*
    if(sinceWifiCheck >= 600000){ //rescan wifi networks every 10 minutes
      wifiScan = true;
    }*/
    if(sinceConnCheck >= 60000){
      if(_ha_en && debugInfo) hadebugDevice(false);
      checkConnection();
      sinceConnCheck = 0;
    }
    if(sinceDebugUpload >= 300000){
      getHeapDebug();
      sinceDebugUpload = 0;
    }
    if(reconncount > 15 || remotehostcount > 60){
      saveResetReason("Rebooting to try fix connections");
      if(saveConfig()){
        syslog("Rebooting to try fix connections", 2);
        setReboot();
>>>>>>> develop
      }
    }
<<<<<<< HEAD
    prevButtonState = M5.Btn.isPressed();
    if (HWSERIAL.available() > 0) {
      String s=HWSERIAL.readStringUntil('!');
      s = s + HWSERIAL.readStringUntil('\n');
      s = s + '\n';
      //Serial.println(s);
      if(trigger_type == 1){
        digitalWrite(TRIGGER, LOW);
        sinceTelegramRequest = 0;
=======
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
>>>>>>> develop
      }
      if(meterError){
        //syslog("Meter reconnected", 0);
        meterError = false;
      }
      splitTelegram(s);
    }
  }
<<<<<<< HEAD
=======
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
>>>>>>> develop
