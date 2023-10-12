/*External integrations to process meter readings*/

void externalIntegrationsBootstrap(){
  /*Put variables that need to be initted at boot here*/
  _key_pushlist = 65534;//511;
  _mbus_pushlist = 136;
  _payload_format = 3; 
  _upload_throttle = 10;
  sinceLastUpload = _upload_throttle*1000;
  if(_wifi_ssid != "") _wifi_STA = true;
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
