boolean scanWifi(){
  syslog("Performing wifi scan", 0);
  int16_t n = WiFi.scanNetworks();
  String savedSSID = wifi_ssid;
  boolean foundSavedSSID = false;
  String buildSSIDlist = "";
  for (int i = 0; i < n; ++i) {
    if(WiFi.SSID(i) != savedSSID){
      buildSSIDlist += "<option value=\"";
      buildSSIDlist += WiFi.SSID(i);
      buildSSIDlist += "\">";
      buildSSIDlist += WiFi.SSID(i);
      buildSSIDlist += "</option>";
    }
    else{
      foundSavedSSID = true;
    }
  }
  buildSSIDlist += "</select>";  
  ssidList = "<select name=\"ssid\">";
  if(foundSavedSSID){
    ssidList += "<option value=\"";
    ssidList += savedSSID;
    ssidList += "\">";
    ssidList += savedSSID;
    ssidList += "</option>";
  }
  ssidList += buildSSIDlist;
  wifiScan = false;
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
    if(!writeFile(SPIFFS, "/test.txt", "Hello ") || !appendFile(SPIFFS, "/test.txt", "World!\r\n")  || !readFile(SPIFFS, "/test.txt")){
      syslog("Could not perform file I/O on SPIFFS", 3);
      spiffsMounted = false;
    }
    else spiffsMounted = true;
    file.close();
  }
  syslog("----------------------------", 1);
  if(spiffsMounted){
    reinit_spiffs = false;
    saveConfig();
  }
  else if(!reinit_spiffs && !spiffsMounted){ //if SPIFFS couldn't be mounted, try a restart first
    reinit_spiffs = true;
    saveConfig();
    delay(500);
    ESP.restart();
  }
}

void initWifi(){
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
      if(spiffsMounted) unitState = 4;
      else unitState = 7;
      MDNS.addService("http", "tcp", 80);
      /*Start NTP time sync*/
      setClock(true);
      printLocalTime(true);
      if(client){
        syslog("Setting up TLS/SSL client", 0);
        client->setUseCertBundle(true);
        // Load certbundle from SPIFFS
        File file = SPIFFS.open("/cert/x509_crt_bundle.bin");
        if(!file || file.isDirectory()) {
            syslog("Could not load cert bundle from SPIFFS", 3);
            bundleLoaded = false;
            unitState = 7;
        }
        // Load loadCertBundle into WiFiClientSecure
        if(file && file.size() > 0) {
            if(!client->loadCertBundle(file, file.size())){
                syslog("WiFiClientSecure: could not load cert bundle", 3);
                bundleLoaded = false;
                unitState = 7;
            }
        }
        file.close();
      } 
      else {
        syslog("Unable to create SSL client", 2);
        unitState = 7;
        httpsError = true;
      }
      if(update_start){
        unitState = -1;
        startUpdate();
      }
      if(update_finish){
        unitState = -1;
        finishUpdate(false);
      }
      if(restore_finish || !spiffsMounted){
        unitState = -1;
        finishUpdate(true);
      }
      if(mqtt_en) setupMqtt();
      sinceConnCheck = 60000;
      server.addHandler(new WebRequestHandler());
      update_autoCheck = true;
      if(update_autoCheck) {
        sinceUpdateCheck = 86400000-60000;
      }
      syslog("Local IP: " + WiFi.localIP().toString(), 0);
    }
    else{
      syslog("Could not connect to the WiFi network", 2);
      wifiError = true;
      wifiSTA = false;
      unitState = 1;
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

void setMeterTime(){
  if(mTimeFound){
    if(!timeSet) syslog("Syncing to to metertime", 0);
    localtime(&dm_timestamp);
    timeSet = true;
  }
}

void checkConnection(){
  if(WiFi.status() != WL_CONNECTED){
    syslog("Lost WiFi connection, trying to reconnect", 2);
    WiFi.disconnect();
    elapsedMillis restartAttemptTime;
    while (WiFi.status() != WL_CONNECTED && restartAttemptTime < 20000) {
      WiFi.reconnect();
    }
    if(restartAttemptTime >= 20000) {
      syslog("Wifi reconnection failed! Trying again in a minute", 3);
      reconncount++;
      wifiError = true;
    }
  }
  if(wifiError && WiFi.status() == WL_CONNECTED){
    wifiError = false;
    syslog("Reconnected to the WiFi network", 0);
    if(!mqtt_en) reconncount = 0;
  }
  if(WiFi.status() == WL_CONNECTED){
    if(mqtt_en){
      connectMqtt();
      if(ha_en && !ha_metercreated) haAutoDiscovery(1);
      else if(ha_en && ha_metercreated) haAutoDiscovery(0);
    }
  }
}

void setReboot(){
  sinceConnCheck = 0;
  syslog("Removing Home Assistant entities", 0);
  haAutoDiscovery(3);
  syslog("Saving configuration", 0);
  saveConfig();
  preferences.end();
  SPIFFS.end();
  rebootInit = true;
  sinceRebootCheck = 0;
  syslog("Rebooting", 2);
}

double round2(double value) {
   return (int)(value * 100 + 0.05) / 100.0;
}
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

void deleteFile(fs::FS &fs, const char * path){
    if(fs.remove(path)){
    } else {
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
