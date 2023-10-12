boolean scanWifi(){
  /*Scan all WiFi networks in range and compile their SSIDs into a JSON array*/
  syslog("Performing wifi scan", 0);
  boolean foundSavedSSID = false;
  int16_t n = WiFi.scanNetworks();
  /*Check if the previously saved SSID is detected.*/
  for (int i = 0; i < n; ++i) {
    if(WiFi.SSID(i) = _wifi_ssid){
      foundSavedSSID = true;
    }
  }
  DynamicJsonDocument doc(1024);
  JsonArray data = doc.createNestedArray("SSIDlist");
  int offset = 0;
  /*If the previously saved SSID is present, put it first in the JSON array so the webmin HTML selects this SSID as the default option to display in the dropdown selection*/
  if(foundSavedSSID){
    data[0]["SSID"] = _wifi_ssid;
    offset = 1;
  }
  for (int i = 0; i < n; ++i) {
    if(WiFi.SSID(i) != _wifi_ssid){
      data[offset]["SSID"] = WiFi.SSID(i);
      offset++;
    }
  }
  serializeJson(doc, ssidList);
  wifiScan = false;
  WiFi.scanDelete();
  return foundSavedSSID;
}

String getHostname(){
  WiFi.macAddress(mac);
  char macbuf[] = "00000";
  String macbufs = "";
  macbufs += String(mac[3], HEX);
  macbufs += String(mac[4], HEX);
  macbufs += String(mac[5], HEX);
  macbufs.toUpperCase();
  macbufs.toCharArray(macbuf, 7);
  apSSID[2] = macbuf[0];
  apSSID[3] = macbuf[1];
  apSSID[4] = macbuf[2];
  apSSID[5] = macbuf[3];
  apSSID[6] = macbuf[4];
  apSSID[7] = macbuf[5];
  return macbufs;
}

void initSPIFFS(){
  /*Initialize SPIFFS*/
  syslog("Mounting SPIFFS... ", 0);
  if(!SPIFFS.begin(true)){
    syslog("Could not mount SPIFFS", 3);
    spiffsMounted = false;
  }
  else{
    /*Check if SPIFFS contains files*/
    syslog("SPIFFS used bytes/total bytes:" + String(SPIFFS.usedBytes()) +"/" + String(SPIFFS.totalBytes()), 0);
    listDir(SPIFFS, "/", 0);
    File file = SPIFFS.open("/index.html"); //test a file
    if(!file || file.isDirectory() || file.size() == 0) {
        syslog("Could not load files from SPIFFS", 3);
        spiffsMounted = false;
    }
    /*Check SPIFFS file I/O*/
    syslog("Testing SPIFFS file I/O... ", 0);
    if(!writeFile(SPIFFS, "/test.txt", "Hello ") || !appendFile(SPIFFS, "/test.txt", "World!\r\n")  || !readFile(SPIFFS, "/test.txt") || !deleteFile(SPIFFS, "/test.txt")){
      syslog("Could not perform file I/O on SPIFFS", 3);
      spiffsMounted = false;
    }
    else spiffsMounted = true;
    file.close();
  }
  syslog("----------------------------", 0);
  if(spiffsMounted){
    _reinit_spiffs = false;
    saveConfig();
  }
  else if(!_reinit_spiffs && !spiffsMounted){ //if SPIFFS couldn't be mounted, try a restart first
    _reinit_spiffs = true;
    saveResetReason("Rebooting to retry SPIFFS access");
    saveConfig();
    delay(500);
    ESP.restart();
  }
}

void initWifi(){
  scanWifi();
  if(_wifi_STA){
    syslog("WiFi mode: station", 1);
    WiFi.mode(WIFI_STA);
    WiFi.begin(_wifi_ssid.c_str(), _wifi_password.c_str());
    WiFi.setHostname("p1dongle");
    elapsedMillis startAttemptTime;
    syslog("Attempting connection to WiFi network " + _wifi_ssid, 0);
    while (WiFi.status() != WL_CONNECTED && startAttemptTime < 20000) {
      delay(200);
      Serial.print(".");
    }
    Serial.println("");
    if(WiFi.status() == WL_CONNECTED){
      syslog("Connected to the WiFi network " + _wifi_ssid, 1);
      MDNS.begin("p1dongle");
      if(spiffsMounted) unitState = 4;
      else unitState = 7;
      /*Add WiFi events to immediately detect WiFi changes*/
      WiFi.onEvent(WiFiEvent, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_LOST_IP);
      WiFi.onEvent(WiFiEvent, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE);
      WiFi.onEvent(WiFiEvent, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
      /*Advertise over mDNS this dongle can be accessed at port 80*/
      MDNS.addService("http", "tcp", 80);
      /*Start NTP time sync*/
      setClock(true);
      printLocalTime(true);
      if(client){
        syslog("Setting up TLS/SSL client", 0);
        //client->setCACertBundle(true);
        // Load certbundle from SPIFFS
        File file = SPIFFS.open("/cert/x509_crt_bundle.bin", "r");
        if(!file || file.isDirectory()) {
            syslog("Could not load cert bundle from SPIFFS", 3);
            //client->setCACertBundle(false);
            bundleLoaded = false;
            unitState = 7;
        }
        // Load loadCertBundle into WiFiClientSecure
        else {
                size_t fileSize = file.size();
                uint8_t *certData = new uint8_t[fileSize];
                file.read(certData, fileSize);
                client->setCACertBundle(certData);
                //delete[] certData;
        }
        file.close();
      } 
      else {
        syslog("Unable to create SSL client", 2);
        //client->setCACertBundle(false);
        unitState = 7;
        httpsError = true;
      }
      if(_update_start){
        unitState = -1;
        startUpdate();
      }
      if(_update_finish){
        unitState = -1;
        finishUpdate(false);
      }
      if(_restore_finish || !spiffsMounted){
        unitState = -1;
        finishUpdate(true);
      }
      if(_mqtt_en) setupMqtt();
      sinceConnCheck = 60000;
      _update_autoCheck = true;
      if(_update_autoCheck) {
        sinceUpdateCheck = 86400000-60000;
      }
      syslog("Local IP: " + WiFi.localIP().toString(), 0);
    }
    else{
      syslog("Could not connect to the WiFi network", 2);
      wifiError = true;
      _wifi_STA = false;
      unitState = 1;
    }
  }
  if(!_wifi_STA){
    syslog("WiFi mode: access point", 1);
    WiFi.mode(WIFI_AP);
    WiFi.softAP("p1dongle");
    dnsServer.start(53, "*", WiFi.softAPIP());
    MDNS.begin("p1dongle");
    syslog("AP set up", 1);
    unitState = 0;
  }
}

String printLocalTime(boolean verbosePrint){
  struct tm timeinfo;
  String(timestring);
  time_t now;
  if(!getLocalTime(&timeinfo)){
    timestring = "";
    if(verbosePrint) syslog("Failed to obtain time from RTC", 2);
    timeSet = false;
  }
  else{
    char timeStringBuff[30];
    strftime(timeStringBuff, sizeof(timeStringBuff), "%Y/%m/%d %H:%M:%S", &timeinfo);
    //if(!timeSet) timestring = "Time set: ";
    timestring = timestring + String(timeStringBuff);
    if(verbosePrint) syslog("Time set: " + timestring, 1);
    timeSet = true;
  }
  return(timestring);
}

unsigned long printUnixTime(){
  time_t now;
  return(time(&now));
}

void setClock(boolean firstSync)
{  
  time_t nowSecs = time(nullptr);
  if(firstSync){
    syslog("Configuring NTP time sync", 0);
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    timeconfigured = true;
  }
}

void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info){
  /*If a change in WiFi happens, skip immediately to checking the connection*/
  Serial.println("Disconnected from WiFi access point");
  Serial.print("WiFi lost connection. Reason: ");
  Serial.println(info.wifi_sta_disconnected.reason);
  sinceConnCheck = 60000;
}

void checkConnection(){
  if(WiFi.status() != WL_CONNECTED){
    syslog("Lost WiFi connection, trying to reconnect", 2);
    WiFi.disconnect();
    wifiError = true;
    mqttClientError = true;
    elapsedMillis restartAttemptTime;
    while (WiFi.status() != WL_CONNECTED && restartAttemptTime < 20000) {
      WiFi.begin(_wifi_ssid.c_str(), _wifi_password.c_str());
    }
    if(restartAttemptTime >= 20000) {
      syslog("Wifi reconnection failed! Trying again in a minute", 3);
      reconncount++;
    }
  }
  if(wifiError && WiFi.status() == WL_CONNECTED){
    wifiError = false;
    syslog("Reconnected to the WiFi network", 0);
    reconncount = 0;
  }
  if(WiFi.status() == WL_CONNECTED){
    wifiRSSI = WiFi.RSSI();
    if(_mqtt_en && !mqttPaused){
      if(mqttPushFails > 5){
        mqttClientError = true;
        syslog("MQTT client connection failed", 4);
        mqttPushFails = 0;
        reconncount++;
      }
      if(mqttHostError) setupMqtt();
      else connectMqtt();
    }
  }
}

void setReboot(){
  sinceConnCheck = 0;
  //syslog("Removing Home Assistant entities", 0);
  //haAutoDiscovery(3); CHECKTHIS
  //syslog("Saving configuration", 0);
  saveConfig();
  //preferences.end();
  SPIFFS.end();
  rebootInit = true;
  sinceRebootCheck = 0;
  syslog("Rebooting", 2);
}

void forcedReset(){
// use the watchdog timer to do a hard restart
// It sets the wdt to 1 second, adds the current process and then starts an
// infinite loop.
  esp_task_wdt_init(1, true);
  esp_task_wdt_add(NULL);
  while(true);  // wait for watchdog timer to be triggered
}

double round2(double value) {
   return (int)(value * 100 + 0.05) / 100.0;
}

/*SPIFFS file utilities*/
void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\r\n", dirname);
    File root = fs.open(dirname);
    if(!root){
        Serial.println("- failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println(" - not a directory");
        return;
    }
    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.path(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

bool writeFile(fs::FS &fs, const char * path, const char * message){
   File file = fs.open(path, FILE_WRITE);
   if(!file){
      return false;
   }
   if(file.print(message)){
      return true;
   }else {
      return false;
   }
}

bool appendFile(fs::FS &fs, const char * path, const char * message){
    File file = fs.open(path, FILE_APPEND);
    if(!file){
        return false;
    }
    if(file.print(message)){
        return true;
    } else {
        return false;
    }
    file.close();
}

bool renameFile(fs::FS &fs, const char * path1, const char * path2){
    if (fs.rename(path1, path2)) {
        return true;
    } else {
        return false;
    }
}

bool deleteFile(fs::FS &fs, const char * path){
    if(fs.remove(path)){
      return true;
    } else {
      return false;
    }
}

int sizeFile(fs::FS &fs, const char * path){
    int fileSize = 0;
    File file = fs.open(path);
    if(!file || file.isDirectory()){
        return fileSize;
    }
    fileSize = file.size();
    file.close();
    return fileSize;
}

bool readFile(fs::FS &fs, const char * path){
    File file = fs.open(path);
    if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
        return false;
    }
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
    return true;
}
