void scanWifi(){
  int n = WiFi.scanNetworks();
  delay(100);
  //Serial.println("");
  //ssidList = "<form method=\"get\" action=\"setap\"><label>SSID:</label><select name=\"ssid\">";
  //String savedSSID = preferences.getString("WIFI_SSID");
  String savedSSID = wifi_ssid;
  //Serial.print("Saved SSID: ");
  //Serial.println(wifi_ssid);
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
      //Serial.println("Found SSID!");
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
  delay(100);
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
}

void printLocalTime(){
  struct tm timeinfo;
  time_t now;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.print("UTC timestamp: ");
  Serial.println(time(&now));
  Serial.print("Human timestamp: ");
  Serial.print(asctime(&timeinfo));
}

void setClock(boolean firstSync)
{
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  time_t nowSecs = time(nullptr);
  if(firstSync){
    Serial.print(F("Waiting for NTP time sync: "));
    int ntpRetries;
    while (nowSecs < 8 * 3600 * 2 && ntpRetries < 10) {
      delay(500);
      ntpRetries++;
      Serial.print(F("."));
      yield();
      nowSecs = time(nullptr);
    }
    Serial.println();
    if(ntpRetries <= 10) {
      Serial.println("Failed to get time through NTP");
      setMeterTime();
    }
    else{
      Serial.println("Time set through NTP");
      struct tm timeinfo;
      gmtime_r(&nowSecs, &timeinfo);
    }
  }
  if (nowSecs > 8 * 3600 * 2) timeSet = true;
  else setMeterTime();
}

void setMeterTime(){
  if(mTimeFound){
    if(!timeSet) Serial.println("Syncing to to metertime");
    localtime(&dm_timestamp);
    timeSet = true;
  }
}

void checkConnection(){
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Lost WiFi connection, reconnecting");
    WiFi.disconnect();
    elapsedMillis restartAttemptTime;
    while (WiFi.status() != WL_CONNECTED && restartAttemptTime < 20000) {
      Serial.print(".");
      WiFi.reconnect();
    }
    Serial.println("");
    if(restartAttemptTime >= 20000) Serial.println("Failed! Trying again in 10s");
    if(WiFi.status() == WL_CONNECTED) Serial.println("Reconnected to the WiFi network");
  }
  else{
    if(mqtt_en){
      connectMqtt();
      if(ha_en && !ha_metercreated) haAutoDiscovery(false);
    }
  }
}

void setBuff(uint8_t Rdata, uint8_t Gdata, uint8_t Bdata)
{
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
