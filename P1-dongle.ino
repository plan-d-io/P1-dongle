#include "rom/rtc.h"
#include "M5Atom.h"
#include <WiFi.h>
#include <WiFiMulti.h>
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
#include "tls.hpp"
#include "FS.h"
#include <Update.h>
#include "ArduinoJson.h"
#include <elapsedMillis.h>

unsigned int fw_ver = 103;
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

//LED state machine vars
uint8_t DisBuff[2 + 5 * 5 * 3];
elapsedMillis ledTime;
boolean ledState = true;
byte unitState = 0;

//General housekeeping vars
unsigned int counter, bootcount, reconncount;
String resetReason, last_reset, last_reset_verbose;
float freeHeap, minFreeHeap, maxAllocHeap;
Preferences preferences;  
String ssidList;
char apSSID[] = "COFY0000";
byte mac[6];
boolean wifiSTA = false;
boolean rebootReq = false;
boolean rebootInit = false;
boolean wifiError, mqttHostError, mqttClientError, mqttWasConnected, httpsError, meterError, eidError, wifiSave, wifiScan, eidSave, mqttSave, haSave, debugInfo, timeconfigured, firstDebugPush, beta_fleet;
String dmPowIn, dmPowCon, dmTotCont1, dmTotCont2, dmTotInt1, dmTotInt2, dmActiveTariff, dmVoltagel1, dmVoltagel2, dmVoltagel3, dmCurrentl1, dmCurrentl2, dmCurrentl3, dmGas, dmText, dmAvDem, dmMaxDemM;
String meterConfig[17];
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
  M5.begin(true, false, true);
  delay(10);
  pinMode(TRIGGER, OUTPUT);
  setBuff(0x00, 0xff, 0x00); //red
  M5.dis.displaybuff(DisBuff);
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
  // Initialize SPIFFS
  syslog("Mounting SPIFFS... ", 0);
  if(!SPIFFS.begin(false)){
    syslog("Could not mount SPIFFS", 3);
    rebootInit = true;
  }
  else{
    spiffsMounted = true;
    syslog("SPIFFS used bytes/total bytes:" + String(SPIFFS.usedBytes()) +"/" + String(SPIFFS.totalBytes()), 0);
  }
  syslog("----------------------------", 1);
  syslog("Digital meter dongle " + String(apSSID) +" V" + String(fw_ver/100.0) + " by plan-d.io", 1);
  if(beta_fleet) syslog("Using development firmware", 2);
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
  //your other setup stuff...
  if(wifiSTA){
    syslog("WiFi mode: station", 1);
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
    WiFi.setHostname("p1dongle");
    elapsedMillis startAttemptTime;
    syslog("Attempting connection to WiFi network " + wifi_ssid, 0);
    while (WiFi.status() != WL_CONNECTED && startAttemptTime < 20000) {
      delay(200);
      Serial.print(".");
    }
    Serial.println("");
    if(WiFi.status() == WL_CONNECTED){
      syslog("Connected to the WiFi network " + wifi_ssid, 1);
      MDNS.begin("p1dongle");
      unitState = 5;
      MDNS.addService("http", "tcp", 80);
      /*Start NTP time sync*/
      setClock(true);
      printLocalTime(true);
      if(client){
        syslog("Setting up TLS/SSL client", 0);
        client->setUseCertBundle(true);
        // Load certbundle from SPIFFS
        File file = SPIFFS.open("/cert/x509_crt_bundle.bin", "r");
        if(!file) {
            syslog("Could not load cert bundle from SPIFFS", 2);
            bundleLoaded = false;
            rebootInit = true;
        }
        // Load loadCertBundle into WiFiClientSecure
        if(file && file.size() > 0) {
            if(!client->loadCertBundle(file, file.size())){
                syslog("WiFiClientSecure: could not load cert bundle", 2);
                bundleLoaded = false;
                rebootInit = true;
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
    WiFi.softAP("p1dongle");
    dnsServer.start(53, "*", WiFi.softAPIP());
    MDNS.begin("p1dongle");
    server.addHandler(new WebRequestHandler()).setFilter(ON_AP_FILTER);//only when requested from AP
    syslog("AP set up", 1);
    unitState = 3;
  }
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
    if(reconncount > 30){
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
    }
  }
  prevButtonState = M5.Btn.isPressed();
  if (HWSERIAL.available() > 0) {
    String s=HWSERIAL.readStringUntil('!');
    s = s + HWSERIAL.readStringUntil('\n');
    s = s + '\n';
    //Serial.println(s);
    if(trigger_type == 1){
      digitalWrite(TRIGGER, LOW);
      sinceTelegramRequest = 0;
    }
    if(meterError){
      //syslog("Meter reconnected", 0);
      meterError = false;
    }
    splitTelegram(s);
  }
}
