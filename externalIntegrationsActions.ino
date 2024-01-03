/*External integrations to process meter readings*/

void externalIntegrationsBootstrap(){
  /*Put variables that need to be initted at boot here*/
  sinceLastUpload = _upload_throttle*1000;
  _key_pushlist = 4294967295;
  if(_wifi_ssid != "") _wifi_STA = true;
  if(_eid_en) lastEIDcheck = EIDcheckInterval-60000;
}

void eidUpload(){
  if(EIDuploadEn && telegramCount > 3 && meterTimestamp > 0){
    syslog("Preparing EID upload", 0);
    String tempOutput;
    String eidOutput = "[";
    DynamicJsonDocument eidData(1024);
    eidData["ts"] = meterTimestamp;
    for(int i = 0; i < sizeof(dsmrKeys)/sizeof(dsmrKeys[0]); i++){
      for(int j = 0; j < sizeof(eidDsmrKeys)/sizeof(eidDsmrKeys[0]); j++){
        if(dsmrKeys[i].dsmrKey == eidDsmrKeys[j][1]){
          if(*dsmrKeys[i].keyFound == true){
            if(dsmrKeys[i].keyType == 5) eidData[eidDsmrKeys[j][0]] = *dsmrKeys[i].keyValueLong;
            else{
              if(fmodf(*dsmrKeys[i].keyValueFloat, 1.0) == 0) eidData[eidDsmrKeys[j][0]] = int(*dsmrKeys[i].keyValueFloat);
              else eidData[eidDsmrKeys[j][0]] = round2(*dsmrKeys[i].keyValueFloat);
            }
          }
        }
      }
    }
    serializeJson(eidData, tempOutput);
    eidData.clear();
    eidOutput += tempOutput;
    eidOutput += ",";
    tempOutput = "";
    eidData["ts"] = maxDemTime;
    eidData["high-pp"] = round2(maxDemM);
    serializeJson(eidData, tempOutput);
    eidData.clear();
    eidOutput += tempOutput;
    tempOutput = "";
    if(totGasCon > 0 && totGasTime > 0){
      eidOutput += ",";
      eidData["ts"] = totGasTime;
      eidData["g"] = round2(totGasCon);
      serializeJson(eidData, tempOutput);
      eidData.clear();
      eidOutput += tempOutput;
      tempOutput = "";
    }
    if(totWatCon > 0 && totWatTime > 0){
      eidOutput += ",";
      eidData["ts"] = totWatTime;
      eidData["w"] = round2(totWatCon);
      serializeJson(eidData, tempOutput);
      eidData.clear();
      eidOutput += tempOutput;
      tempOutput = "";
    }
    if(totHeatCon > 0 && totHeatTime > 0){
      eidOutput += ",";
      eidData["ts"] = totHeatTime;
      eidData["w"] = round2(totHeatCon);
      serializeJson(eidData, tempOutput);
      eidData.clear();
      eidOutput += tempOutput;
      tempOutput = "";
    }
    eidOutput += "]";
    if(httpDebug) Serial.println(eidOutput);
    clientSecureBusy = true;
    if(_mqtt_tls){
      if(mqttclientSecure.connected()){
        syslog("Disconnecting TLS MQTT connection", 0);
        String mqtt_topic = "plan-d/" + String(apSSID);
        mqttclientSecure.publish(mqtt_topic.c_str(), "offline", true);
        mqttclientSecure.disconnect();
        mqttPaused = true;
      }
    }
    if(bundleLoaded){
      syslog("Performing EID upload", 0);
      syslog("Connecting to " + EIDwebhookUrl, 0);
      if (https.begin(*client, EIDwebhookUrl)) {
        https.addHeader("Authorization", EIDauthorization);
        https.addHeader("x-twin-id", EIDxtwinid);
        https.addHeader("Content-Type", "application/json");
        int httpCode = https.POST(eidOutput);
        if (httpCode > 0) {
          if(httpCode == 200 || httpCode == 201){
            syslog("EID upload succeeded", 0);
          }
          secureClientError = 0;
          lastEIDupload = 0;
          if(_rebootSecure > 0){
            _rebootSecure = 0;
            saveConfig();
          }
        }
        else{
          syslog("Could not connect to EID, HTTPS code " + String(https.errorToString(httpCode)), 2);
          lastEIDupload = EIDuploadInterval - 60000;
          secureClientError++;
        }
        https.end();
        while(client->read() != -1);
        client->stop();
      }
      else {
        syslog("Unable to connect to EID", 2);
        lastEIDupload = EIDuploadInterval - 60000;
      }
      clientSecureBusy = false;
      if(mqttPaused){
        sinceConnCheck = 60000;
        mqttPaused = false;
        mqttWasPaused = true;
      }
    }
  }
}

void eidHello(){
  if(_eid_en){
    syslog("Preparing EID hello", 0);
    clientSecureBusy = true;
    if(_mqtt_tls){
      if(mqttclientSecure.connected()){
        syslog("Disconnecting TLS MQTT connection", 0);
        String mqtt_topic = "plan-d/" + String(apSSID);
        mqttclientSecure.publish(mqtt_topic.c_str(), "offline", true);
        mqttclientSecure.disconnect();
        mqttPaused = true;
      }
    }
    if(client && bundleLoaded){
      syslog("Performing EID hello", 0);
      String checkUrl = "https://hooks.energyid.eu/hello";
      syslog("Connecting to " + checkUrl, 0);
      if (https.begin(*client, checkUrl)) {
        https.addHeader("X-Provisioning-Key", _eid_provkey);
        https.addHeader("X-Provisioning-Secret", _eid_provsec);
        https.addHeader("Content-Type", "application/json");
        int httpCode = https.POST(eidHelloMsg());
        if (httpCode > 0) {
          if(httpDebug) Serial.println(httpCode);
          String payload = https.getString();
          if(httpDebug) Serial.println(payload);
          if(httpCode == 200 || httpCode == 201){
            DynamicJsonDocument doc(1048);
            deserializeJson(doc, payload);
            JsonObject obj = doc.as<JsonObject>();
            JsonVariant jclaimCode = obj["claimCode"];
            JsonVariant jclaimUrl = obj["claimUrl"];
            JsonVariant jallowedInterval = obj["webhookPolicy"]["allowedInterval"];
            String claimUrl;
            if(!jallowedInterval.isNull()){
              String webhookUrl = obj["webhookUrl"];
              EIDwebhookUrl = webhookUrl;
              String authorization = obj["headers"]["authorization"];
              EIDauthorization = authorization;
              String xtwinid = obj["headers"]["x-twin-id"];
              EIDxtwinid = xtwinid ;
              String allowedInterval = obj["webhookPolicy"]["allowedInterval"];
              bool daily = false;
              int multi = 1000;
              int len = allowedInterval.length();
              char intervalUnit = allowedInterval.charAt(len-1);
              if(intervalUnit == 'S' || intervalUnit == 's') multi = 1000;
              else if(intervalUnit == 'M' || intervalUnit == 'm') multi = 60* 1000;
              else if(intervalUnit == 'H' || intervalUnit == 'h') multi = 60 * 60* 1000;
              else if(intervalUnit == 'D' || intervalUnit == 'd'){
                multi = 24 * 60 * 60* 1000;
                daily = true;
              }
              else multi = 1000;
              if(daily) allowedInterval = allowedInterval.substring(1, len-1);
              else allowedInterval = allowedInterval.substring(2, len-1);
              EIDuploadInterval = allowedInterval.toInt();
              EIDuploadInterval = EIDuploadInterval * multi;
              EIDcheckInterval = 24*60*1000;
              EIDuploadEn = true;
              eidUploadInterval = allowedInterval + String(intervalUnit);
              if(intervalUnit == 'S' || intervalUnit == 'M' || intervalUnit == 'H') eidUploadInterval.toLowerCase();
              if(EIDuploadEn) syslog("EID can upload, setting interval to " + allowedInterval + String(intervalUnit), 0);
              lastEIDupload = EIDuploadInterval;
              configBuffer = returnConfig();
            }
            else{
              EIDuploadEn = false;
              EIDcheckInterval = 150000;
              syslog("EID cannot yet upload", 0);
            }
            if(!jclaimCode.isNull()){
              String claimTest = obj["claimCode"];
              _eidclaim = claimTest;
            }
            secureClientError = 0;
            if(_rebootSecure > 0){
              _rebootSecure = 0;
              saveConfig();
            }
          }
        }
        else{
          syslog("Could not connect to EID, HTTPS code " + String(https.errorToString(httpCode)), 2);
          secureClientError++;
          EIDuploadEn = false;
          EIDcheckInterval = 150000;
        }
        https.end();
        while(client->read() != -1);
        client->stop();
      }
      else {
        syslog("Unable to connect to EID", 2);
        EIDuploadEn = false;
        EIDcheckInterval = 150000;
      }
      clientSecureBusy = false;
      if(mqttPaused){
        sinceConnCheck = 60000;
        mqttPaused = false;
        mqttWasPaused = true;
      }
    }
  }
  lastEIDcheck = 0;
}

String eidHelloMsg(){
  String jsonOutput;
  DynamicJsonDocument doc(1024);
  doc["claimCode"] = _eidclaim;
  doc["deviceId"] = _uuid;
  doc["deviceName"] = "P1 dongle by Plan-D";
  doc["firmwareVersion"] = String(fw_ver/100.0);
  doc["ipAddress"] = WiFi.localIP().toString();
  doc["lcoalDeviceUrl"] = "http://" + WiFi.localIP().toString();
  serializeJson(doc, jsonOutput);
  if(httpDebug) Serial.println(jsonOutput);
  return jsonOutput;
}
