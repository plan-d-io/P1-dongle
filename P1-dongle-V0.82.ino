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

unsigned int fw_ver = 82;
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

elapsedMillis sinceConnCheck, sinceUpdateCheck, sinceClockCheck, sinceLastUpload, sinceEidUpload;

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
String resetReason;
float freeHeap, minFreeHeap, maxAllocHeap;
Preferences preferences;  
String ssidList;
char apSSID[] = "COFY0000";
byte mac[6];

boolean wifiSTA = false;
boolean rebootReq = false;
boolean wifiError, mqttHostError, mqttClientError, httpsError, meterError, eidError, wifiSave, eidSave, mqttSave, haSave, debugInfo;
boolean timeSet, mTimeFound;
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
  setBuff(0x00, 0xff, 0x00); //red
  M5.dis.displaybuff(DisBuff);
  Serial.begin(115200);
  HWSERIAL.begin(115200, SERIAL_8N1, 21, 25);
  delay(500);
  Serial.println();
  Serial.println("Digital meter dongle V" + String(fw_ver/100.0) + " by plan-d.io");
  getHostname();
  Serial.print("Unit name: ");
  Serial.println(apSSID);
  preferences.begin("cofy-config");
  delay(100);
  initConfig();
  delay(100);
  restoreConfig();
  // Initialize SPIFFS
  Serial.print("Mounting SPIFFS... ");
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  else{
    Serial.print("Used bytes/total bytes:" );
    Serial.print(SPIFFS.usedBytes());
    Serial.print("/");
    Serial.println(SPIFFS.totalBytes());
  }
  bootcount = bootcount + 1;
  Serial.print("Boot count: ");
  Serial.println(bootcount);
  saveBoots();
  Serial.print("Last reset reason: ");
  get_reset_reason(rtc_get_reset_reason(0));
  Serial.println(resetReason);
  debugInfo = true;
  delay(100);
  //your other setup stuff...
  if(wifiSTA){
    Serial.println("WiFi mode: station");
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
    WiFi.setHostname(apSSID);
    elapsedMillis startAttemptTime;
    Serial.print("Attempting connection to WiFi network ");
    Serial.print(wifi_ssid);
    while (WiFi.status() != WL_CONNECTED && startAttemptTime < 20000) {
      delay(200);
      Serial.print(".");
    }
    Serial.println("");
    if(WiFi.status() == WL_CONNECTED){
      Serial.println("Connected to the WiFi network");
      MDNS.begin("apSSID");
      unitState = 5;
      /*Start NTP time sync*/
      setClock(true);
      printLocalTime();
      if(client){
        Serial.println("Congfiguring SSL client");
        client->setUseCertBundle(true);
        // Load certbundle from SPIFFS
        File file = SPIFFS.open("/cert/x509_crt_bundle.bin", "r");
        if(!file) {
            Serial.println("Could not load cert bundle from SPIFFS");
            bundleLoaded = false;
        }
        // Load loadCertBundle into WiFiClientSecure
        if(file && file.size() > 0) {
            if(!client->loadCertBundle(file, file.size())){
                Serial.println("WiFiClientSecure: could not load cert bundle");
                bundleLoaded = false;
            }
        }
        file.close();
      } else {
        Serial.println("Unable to create secure client");
        unitState = 5;
        httpsError = true;
      }
      if(update_start){
        Serial.println("Starting upgrade");
        startUpdate();
      }
      else if (update_finish){
        Serial.println("Finishing upgrade");
        finishUpdate();
      }
      if(mqtt_en) setupMqtt();
      if(update_finish) finishUpdate();
      server.addHandler(new WebRequestHandler());
      update_autoCheck = true;
      if(update_autoCheck) {
        sinceUpdateCheck = 86400000-60000;
      }
      if(eid_en) sinceEidUpload = 15*60*900000;
      Serial.print("Local IP: ");
      Serial.println(WiFi.localIP());
    }
    else{
      Serial.println("Could not connect to the WiFi network");
      wifiError = true;
      wifiSTA = false;
    }
  }
  if(!wifiSTA){
    Serial.println("WiFi mode: access point");
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
  if(!wifiSTA){
    dnsServer.processNextRequest();
    if(!timeSet) setMeterTime();
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
      sinceUpdateCheck = 0;
    }
    if(!timeSet) setClock(false);
    if(sinceClockCheck >= 3600){
      setClock(false);
      sinceClockCheck = 0;
    }
    if(sinceConnCheck >= 10000){
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
    prevButtonState = M5.Btn.isPressed();
  }
  if (HWSERIAL.available() > 0) {
    String s=HWSERIAL.readStringUntil('!');
    s = s + HWSERIAL.readStringUntil('\n');
    s = s + '\n';
    splitTelegram(s);
  }
}
