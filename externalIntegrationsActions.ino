/*External integrations to process meter readings*/

void externalIntegrationsBootstrap(){
  /*Put variables that need to be initted at boot here*/
  _key_pushlist = 65534;//511;
  _mbus_pushlist = 136;
  _payload_format = 3; 
  _upload_throttle = 10;
  sinceLastUpload = _upload_throttle*1000;
  if(_wifi_ssid != "") _wifi_STA = true;
  if(_eid_en) lastEIDcheck = EIDcheckInterval;
}

bool realtoUpload(){
  if(lastRealtoUpload > _realtoThrottle*1000){
    String tempTopic = _mqtt_prefix.substring(0, _mqtt_prefix.length()-1);
    pubMqtt(tempTopic, "online", false);
    DynamicJsonDocument realtoData(1024);
    realtoData["timestamp"] = meterTimestamp;
    for(int i = 0; i < sizeof(dsmrKeys)/sizeof(dsmrKeys[0]); i++){
      for(int j = 0; j < sizeof(realtoKeys)/sizeof(realtoKeys[0]); j++){
        if(dsmrKeys[i].dsmrKey == realtoKeys[j]){
          if(*dsmrKeys[i].keyFound == true){
            realtoData[realtoKeys[j]]["value"] = *dsmrKeys[i].keyValueFloat;
            realtoData[realtoKeys[j]]["unit"] = getUnit(dsmrKeys[i].deviceType);
          }
        }
      }
    }
    for(int i = 0; i < sizeof(mbusMeter)/sizeof(mbusMeter[0]); i++){
      for(int j = 0; j < sizeof(realtoKeys)/sizeof(realtoKeys[0]); j++){
        if(mbusMeter[i].mbusKey == realtoKeys[j]){
          if(mbusMeter[i].keyFound == true){
            realtoData[realtoKeys[j]]["value"] = mbusMeter[i].keyValueFloat;
            if(mbusMeter[i].type == 3 || mbusMeter[i].type == 7) realtoData[realtoKeys[j]]["unit"] = "m³";
            else if (mbusMeter[i].type == 7) realtoData[realtoKeys[j]]["unit"] = "kWh";
          }
        }
      }
    }
    tempTopic = tempTopic + "/data/readings";
    String realtoOutput;
    serializeJson(realtoData, realtoOutput);
    /*Push the JSON string over MQTT*/
    Serial.print(tempTopic);
    Serial.print(" ");
    Serial.println(realtoOutput);
    if(pubMqtt(tempTopic, realtoOutput, "false")){
      lastRealtoUpload = 0;
      realtoUploadTries = 0;
    }
    else{
      /*If MQTT push failed, try again in 5 sec*/
      lastRealtoUpload = _realtoThrottle - 5000;
      realtoUploadTries++;
    }
    /*Give up after five tries, trying again in five minutes*/
    if(realtoUploadTries > 5){
      lastRealtoUpload = 5*5000; //to compensate for the five retry delays
      realtoUploadTries = 0;
    }
  }
  return true;
}

void eidUpload(){
  if(EIDuploadEn && meterTimestamp > 0){
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
    for(int i = 0; i < sizeof(mbusMeter)/sizeof(mbusMeter[0]); i++){
      if(mbusMeter[i].keyFound == true){
        if(mbusMeter[i].type == 3) eidData["g"] = round2(mbusMeter[i].keyValueFloat);
        else if(mbusMeter[i].type == 7) eidData["w"] = round2(mbusMeter[i].keyValueFloat);
        else if(mbusMeter[i].type == 4) eidData["h"] = round2(mbusMeter[i].keyValueFloat);
        eidData["ts"] = mbusMeter[i].keyTimeStamp;
        serializeJson(eidData, tempOutput);
        eidData.clear();
        eidOutput += tempOutput;
        eidOutput += ",";
        tempOutput = "";
      }
    }
    eidData["ts"] = maxDemTime;
    eidData["high-pp"] = round2(maxDemM);
    serializeJson(eidData, tempOutput);
    eidOutput += tempOutput;
    eidOutput += "]";
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
        }
        else{
          syslog("Could not connect to EID, HTTPS code " + String(https.errorToString(httpCode)), 2);
        }
        https.end();
        client->stop();
      }
      else {
        syslog("Unable to connect to EID", 2);
      }
      clientSecureBusy = false;
      if(mqttPaused){
        sinceConnCheck = 10000;
        mqttPaused = false;
      }
    }
    lastEIDupload = 0;
  }
}

void eidHello(){
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
  if(bundleLoaded){
    syslog("Performing EID hello", 0);
    String checkUrl = "https://hooks.energyid.eu/hello";
    Serial.println(_eid_provkey);
    Serial.println(_eid_provsec);
    syslog("Connecting to " + checkUrl, 0);
    if (https.begin(*client, checkUrl)) {  
      https.addHeader("X-Provisioning-Key", _eid_provkey);
      https.addHeader("X-Provisioning-Secret", _eid_provsec);
      https.addHeader("Content-Type", "application/json");
      int httpCode = https.POST(eidHelloMsg());
      if (httpCode > 0) {
        Serial.println(httpCode);
        String payload = https.getString();
        Serial.println(payload);
        if(httpCode == 200 || httpCode == 201){
          DynamicJsonDocument doc(1048);
          deserializeJson(doc, payload);
          JsonObject obj = doc.as<JsonObject>();
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
            int multi = 1;
            int len = allowedInterval.length();
            char intervalUnit = allowedInterval.charAt(len-1);
            if(intervalUnit == 'S' || intervalUnit == 's') multi = 1000;
            else if(intervalUnit == 'M' || intervalUnit == 'm'){
              multi = 60000;
            }
            allowedInterval = allowedInterval.substring(2, len);
            EIDuploadInterval = allowedInterval.toInt();
            EIDuploadInterval = EIDuploadInterval * multi;
            EIDcheckInterval = 24*60*1000;
            EIDuploadEn = true;
            if(EIDuploadEn) syslog("EID can upload, setting interval to " + allowedInterval, 0);
            lastEIDupload = EIDuploadInterval;
          }
          else{
            EIDuploadEn = false;
            EIDcheckInterval = 150000;
            syslog("EID cannot yet upload", 0);
          }
        }
      }
      else{
        syslog("Could not connect to EID, HTTPS code " + String(https.errorToString(httpCode)), 2);
        EIDuploadEn = false;
        EIDcheckInterval = 150000;
      }
      https.end();
      client->stop();
    }
    else {
      syslog("Unable to connect to EID", 2);
      EIDuploadEn = false;
      EIDcheckInterval = 150000;
    }
    lastEIDcheck = 0;
    clientSecureBusy = false;
    if(mqttPaused){
      sinceConnCheck = 10000;
      mqttPaused = false;
    }
  }
}

String eidHelloMsg(){
  String jsonOutput;
  DynamicJsonDocument doc(1024);
  doc["claimCode"] = String(apSSID);
  doc["deviceId"] = String(apSSID);
  doc["deviceName"] = "P1 dongle by Plan-D";
  doc["firmwareVersion"] = String(fw_ver/100.0);
  doc["ipAddress"] = WiFi.localIP().toString();
  doc["lcoalDeviceUrl"] = "http://" + WiFi.localIP().toString();
  serializeJson(doc, jsonOutput);
  return jsonOutput;
}
