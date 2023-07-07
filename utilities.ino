boolean scanWifi(){
  syslog("Performing wifi scan", 0);
  int16_t n = WiFi.scanNetworks();
  String savedSSID = _wifi_ssid;
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
  macbufs += String(mac[4], HEX);
  macbufs += String(mac[5], HEX);
  macbufs.toUpperCase();
  macbufs.toCharArray(macbuf, 5);
  apSSID[4] = macbuf[0];
  apSSID[5] = macbuf[1];
  apSSID[6] = macbuf[2];
  apSSID[7] = macbuf[3];
  return macbufs;
}

void initSPIFFS(){
  // Initialize SPIFFS //move this to a utility function
  syslog("Mounting SPIFFS... ", 0);
  if(!SPIFFS.begin(true)){
    syslog("Could not mount SPIFFS", 3);
  }
  else{
    syslog("SPIFFS used bytes/total bytes:" + String(SPIFFS.usedBytes()) +"/" + String(SPIFFS.totalBytes()), 0);
    listDir(SPIFFS, "/", 0);
    File file = SPIFFS.open("/index.html");
    if(!file || file.isDirectory() || file.size() == 0) {
        syslog("Could not load files from SPIFFS", 3);
    }
    else spiffsMounted = true;
    file.close();
  }
  syslog("----------------------------", 1);
  syslog("Digital meter dongle " + String(apSSID) +" V" + String(fw_ver/100.0) + " by plan-d.io", 1);
}

void initWifi(){
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
      unitState = 5;
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
        }
        // Load loadCertBundle into WiFiClientSecure
        if(file && file.size() > 0) {
            if(!client->loadCertBundle(file, file.size())){
                syslog("WiFiClientSecure: could not load cert bundle", 3);
                bundleLoaded = false;
            }
        }
        file.close();
      } 
      else {
        syslog("Unable to create SSL client", 2);
        unitState = 5;
        httpsError = true;
      }
      if(_update_start){
        startUpdate();
      }
      if(_update_finish){
        finishUpdate(false);
      }
      if(_restore_finish || !spiffsMounted){
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
    }
  }
  if(!_wifi_STA){
    syslog("WiFi mode: access point", 1);
    WiFi.mode(WIFI_AP);
    WiFi.softAP("p1dongle");
    dnsServer.start(53, "*", WiFi.softAPIP());
    MDNS.begin("p1dongle");
    syslog("AP set up", 1);
    unitState = 3;
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
    localtime(&dm_timestamp); //CHECK THIS NOT IMPLEMENTED IN dsmrTelegram YET
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
    if(!_mqtt_en) reconncount = 0;
  }
  if(WiFi.status() == WL_CONNECTED){
    if(_mqtt_en){
      connectMqtt();
      //if(_ha_en && !ha_metercreated) haAutoDiscovery(1); CHECK THIS
      //else if(_ha_en && ha_metercreated) haAutoDiscovery(0);
    }
  }
}

void setReboot(){
  sinceConnCheck = 0;
  syslog("Removing Home Assistant entities", 0);
  //haAutoDiscovery(3); CHECKTHIS
  syslog("Saving configuration", 0);
  saveConfig();
  preferences.end();
  SPIFFS.end();
  rebootInit = true;
  sinceRebootCheck = 0;
  syslog("Rebooting", 2);
}

void setBuff(uint8_t Rdata, uint8_t Gdata, uint8_t Bdata)
{
    //Serial.println("setting LED");
    DisBuff[0] = 0x05;
    DisBuff[1] = 0x05;
    for (int i = 0; i < 25; i++)
    {
        DisBuff[2 + i * 3 + 0] = Rdata;
        DisBuff[2 + i * 3 + 1] = Gdata;
        DisBuff[2 + i * 3 + 2] = Bdata;
    }
}

void blinkLed(){
  if(ledTime >= 300){
    //Serial.println(unitState);
    if(ledState){
      if(unitState == 1 || unitState == 3 ||unitState == 5) setBuff(0x00, 0x00, 0x00);
    }
    else{
      if(unitState <= 1) setBuff(0xff, 0x00, 0x00);
      else if(unitState == 2 || unitState == 3) setBuff(0xff, 0x40, 0x00);
      else setBuff(0x00, 0x00, 0x40);
    }
    ledState = !ledState;
    ledTime = 0;
    M5.dis.displaybuff(DisBuff);
  }
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
