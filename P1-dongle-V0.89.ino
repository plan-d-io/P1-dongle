#include "rom/rtc.h"
#include <M5Atom.h>
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
#include "tls.hpp"
#include "FS.h"
#include <Update.h>
#include "ArduinoJson.h"
#include <elapsedMillis.h>

unsigned int fw_ver = 89;
unsigned int onlineVersion, fw_new;
DNSServer dnsServer;
AsyncWebServer server(80);
WiFiClient wificlient;
PubSubClient mqttclient(wificlient);
WiFiClientSecure *client = new WiFiClientSecure;
PubSubClient mqttclientSecure(*client);
HTTPClient https;
bool bundleLoaded = true;
bool clientSecureBusy;

#define HWSERIAL Serial1
#define TRIGGER 25

elapsedMillis sinceConnCheck, sinceUpdateCheck, sinceClockCheck, sinceLastUpload, sinceEidUpload, sinceLastWebRequest, sinceRebootCheck, sinceMeterCheck, sinceWifiCheck, sinceTelegramRequest;

float totConDay, totConNight, totCon, totInDay, totInNight, totIn, totPowCon, totPowIn, netPowCon, totGasCon, volt1, volt2, volt3;
RTC_NOINIT_ATTR float totConToday, totConYesterday, gasConToday, gasConYesterday;
struct tm dm_time;  // dm time elements structure
time_t dm_timestamp; // dm timestamp
struct tm mb1_time;  // mbus1 time elements structure
time_t mb1_timestamp; // mbus1 timestamp
int prevDay = -1;

uint8_t DisBuff[2 + 5 * 5 * 3];
elapsedMillis ledTime;
boolean ledState = true;
byte unitState = 0;

unsigned int counter, bootcount;
String resetReason, last_reset, last_reset_verbose;
float freeHeap, minFreeHeap, maxAllocHeap;
Preferences preferences;  
String ssidList;
char apSSID[] = "COFY0000";
byte mac[6];

boolean wifiSTA = false;
boolean rebootReq = false;
boolean rebootInit = false;
boolean wifiError, mqttHostError, mqttClientError, mqttWasConnected, httpsError, meterError, eidError, wifiSave, eidSave, mqttSave, haSave, debugInfo, timeconfigured, firstDebugPush;
String dmActiveTariff, dmVoltagel1, dmVoltagel2, dmVoltagel3, dmCurrentl1, dmCurrentl2, dmCurrentl3, dmGas, dmText;
String meterConfig[15];
int dsmrVersion, trigger_type, trigger_interval;
boolean timeSet, mTimeFound, spiffsMounted;
String wifi_ssid, wifi_password;
String mqtt_host, mqtt_id, mqtt_user, mqtt_pass;
uint8_t prevButtonState = false;
boolean configSaved, resetWifi, resetAll;
boolean mqtt_en, mqtt_tls, mqtt_auth;
boolean update_autoCheck, update_auto, updateAvailable, update_start, update_finish, eid_en, ha_en, ha_metercreated;
unsigned int mqtt_port;
unsigned long upload_throttle;
String eid_webhook;

void setup(){
  M5.begin(true, true, true);
  delay(10);
  pinMode(TRIGGER, OUTPUT);
  setBuff(0x00, 0xff, 0x00); //red
  M5.dis.displaybuff(DisBuff);
  Serial.begin(115200);
  HWSERIAL.begin(115200, SERIAL_8N1, 21, 27);
  delay(500);
  getHostname();
  Serial.println();
  syslog("Digital meter dongle booting", 0);
  preferences.begin("cofy-config");
  delay(100);
  initConfig();
  delay(100);
  restoreConfig();
  // Initialize SPIFFS
  syslog("Mounting SPIFFS... ", 0);
  if(!SPIFFS.begin(true)){
    syslog("Could not mount SPIFFS", 3);
    return;
  }
  else{
    spiffsMounted = true;
    syslog("SPIFFS used bytes/total bytes:" + String(SPIFFS.usedBytes()) +"/" + String(SPIFFS.totalBytes()), 0);
  }
  syslog("----------------------------", 1);
  syslog("Digital meter dongle " + String(apSSID) +" V" + String(fw_ver/100.0) + " by plan-d.io", 1);
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
  delay(100);
  //your other setup stuff...
  if(wifiSTA){
    syslog("WiFi mode: station", 1);
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
    WiFi.setHostname(apSSID);
    elapsedMillis startAttemptTime;
    syslog("Attempting connection to WiFi network " + wifi_ssid, 0);
    while (WiFi.status() != WL_CONNECTED && startAttemptTime < 20000) {
      delay(200);
      Serial.print(".");
    }
    Serial.println("");
    if(WiFi.status() == WL_CONNECTED){
      syslog("Connected to the WiFi network " + wifi_ssid, 1);
      MDNS.begin("apSSID");
      unitState = 5;
      /*Start NTP time sync*/
      setClock(true);
      printLocalTime(true);
      if(client){
        syslog("Setting up SSL client", 0);
        client->setUseCertBundle(true);
        // Load certbundle from SPIFFS
        File file = SPIFFS.open("/cert/x509_crt_bundle.bin", "r");
        if(!file) {
            syslog("Could not load cert bundle from SPIFFS", 2);
            bundleLoaded = false;
        }
        // Load loadCertBundle into WiFiClientSecure
        if(file && file.size() > 0) {
            if(!client->loadCertBundle(file, file.size())){
                syslog("WiFiClientSecure: could not load cert bundle", 2);
                bundleLoaded = false;
            }
        }
        file.close();
      } else {
        syslog("Unable to create SSL client", 2);
        unitState = 5;
        httpsError = true;
      }
      if(update_start){
        startUpdate();
      }
      if(update_finish){
        finishUpdate();
      }
      if(mqtt_en) setupMqtt();
      sinceConnCheck = 60000;
      //if(update_finish) finishUpdate();
      server.addHandler(new WebRequestHandler());
      update_autoCheck = true;
      if(update_autoCheck) {
        sinceUpdateCheck = 86400000-60000;
      }
      if(eid_en) sinceEidUpload = 15*60*900000;
      syslog("Local IP: " + WiFi.localIP().toString(), 0);
    }
    else{
      syslog("Could not connect to the WiFi network", 2);
      wifiError = true;
      wifiSTA = false;
    }
  }
  if(!wifiSTA){
    syslog("WiFi mode: access point", 1);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apSSID);
    dnsServer.start(53, "*", WiFi.softAPIP());
    MDNS.begin("apSSID");
    server.addHandler(new WebRequestHandler()).setFilter(ON_AP_FILTER);//only when requested from AP
    //Serial.println("AP set up");
    unitState = 3;
  }
  scanWifi();
  server.begin();
}

void loop(){
  blinkLed();
  if(sinceRebootCheck > 2000){
    if(rebootInit) ESP.restart();
    sinceRebootCheck = 0;
  }
  if(sinceMeterCheck > 60000){
    syslog("Meter disconnected", 2);
    meterError = true;
    sinceMeterCheck = 0;
  }
  if(trigger_type == 1){
    if(sinceTelegramRequest >= trigger_interval *1000){
      //Serial.println("Asserting request line");
      digitalWrite(TRIGGER, HIGH);
      sinceTelegramRequest = 0;
    }
  }
  if(!wifiSTA){
    dnsServer.processNextRequest();
    if(!timeSet) setMeterTime();
    if(sinceWifiCheck >= 300000){
      if(scanWifi()) setReboot();
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
    if(wifiError || mqttHostError || mqttClientError || httpsError || meterError || eidError) unitState = 5;
    else unitState = 4;
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
    }
  }
  prevButtonState = M5.Btn.isPressed();
  if (HWSERIAL.available() > 0) {
    String s=HWSERIAL.readStringUntil('!');
    s = s + HWSERIAL.readStringUntil('\n');
    s = s + '\n';
    if(trigger_type == 1){
      digitalWrite(TRIGGER, LOW);
      sinceTelegramRequest = 0;
    }
    splitTelegram(s);
  }
}
