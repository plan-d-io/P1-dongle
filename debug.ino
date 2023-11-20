void get_reset_reason(RESET_REASON reason){
  switch ( reason){
    case 1 : resetReason = "POWERON_RESET";break;          /**<1,  Vbat power on reset*/
    case 3 : resetReason = "SW_RESET";break;               /**<3,  Software reset digital core*/
    case 4 : resetReason = "OWDT_RESET";break;             /**<4,  Legacy watch dog reset digital core*/
    case 5 : resetReason = "DEEPSLEEP_RESET";break;        /**<5,  Deep Sleep reset digital core*/
    case 6 : resetReason = "SDIO_RESET";break;             /**<6,  Reset by SLC module, reset digital core*/
    case 7 : resetReason = "TG0WDT_SYS_RESET";break;       /**<7,  Timer Group0 Watch dog reset digital core*/
    case 8 : resetReason = "TG1WDT_SYS_RESET";break;       /**<8,  Timer Group1 Watch dog reset digital core*/
    case 9 : resetReason = "RTCWDT_SYS_RESET";break;       /**<9,  RTC Watch dog Reset digital core*/
    case 10 : resetReason = "INTRUSION_RESET";break;       /**<10, Instrusion tested to reset CPU*/
    case 11 : resetReason = "TGWDT_CPU_RESET";break;       /**<11, Time Group reset CPU*/
    case 12 : resetReason = "SW_CPU_RESET";break;          /**<12, Software reset CPU*/
    case 13 : resetReason = "RTCWDT_CPU_RESET";break;      /**<13, RTC Watch dog Reset CPU*/
    case 14 : resetReason = "EXT_CPU_RESET";break;         /**<14, for APP CPU, reseted by PRO CPU*/
    case 15 : resetReason = "RTCWDT_BROWN_OUT_RESET";break;/**<15, Reset when the vdd voltage is not stable*/
    case 16 : resetReason = "RTCWDT_RTC_RESET";break;      /**<16, RTC Watch dog reset digital core and rtc module*/
    default : resetReason = "NO_MEAN";
  }
}

void getHeapDebug(){
  freeHeap = ESP.getFreeHeap()/1000.0;
  minFreeHeap = ESP.getMinFreeHeap()/1000.0;
  maxAllocHeap = ESP.getMaxAllocHeap()/1000.0;
  Serial.println(freeHeap);
  if(_mqtt_en && debugInfo) pushDebugValues();
}

void pushDebugValues(){
  time_t now;
  unsigned long dtimestamp = time(&now);
  for(int i = 0; i < 11; i++){
    String chanName = "";
    String dtopic = "";
    DynamicJsonDocument doc(1024);
    if(i == 0){
      chanName = "reboots";
      doc["friendly_name"] = "Reboots";
      doc["value"] = _bootcount;
    }
    else if(i == 1){
      chanName = "last_reset_reason_hw";
      doc["friendly_name"] = "Last reset reason (hardware)";
      doc["value"] = resetReason;
    }
    else if(i == 2){
      chanName = "free_heap_size";
      doc["friendly_name"] = "Free heap size";
      doc["unit_of_measurement"] = "kB";
      doc["value"] = freeHeap;
    }
    else if(i == 3){
      chanName = "max_allocatable_block";
      doc["friendly_name"] = "Allocatable block size";
      doc["unit_of_measurement"] = "kB";
      doc["value"] = maxAllocHeap;
    }
    else if(i == 4){
      chanName = "min_free_heap";
      doc["friendly_name"] = "Lowest free heap size";
      doc["unit_of_measurement"] = "kB";
      doc["value"] = minFreeHeap;
    }
    else if(i == 5){
      chanName = "last_reset_reason_fw";
      doc["friendly_name"] = "Last reset reason (firmware)";
      doc["value"] = _last_reset;
    }
    else if(i == 6){
      chanName = "ip";
      doc["friendly_name"] = "IP";
      doc["value"] = WiFi.localIP().toString();
    }
    else if(i == 7){
      chanName = "firmware";
      doc["friendly_name"] = "Firmware";
      doc["value"] =  fw_ver/100.0;
    }
    else if(i == 8){
      chanName = "release_channel";
      doc["friendly_name"] = "Release channel";
      if(_alpha_fleet) doc["value"] = "alpha";
      else if(_dev_fleet) doc["value"] = "development";
      else doc["value"] = "main";
    }
    else if(i == 9){
      chanName = "email";
      doc["friendly_name"] = "Email";
      doc["value"] = _user_email;
    }
    else if(i == 10){
      chanName = "rssi";
      doc["friendly_name"] = "RSSI";
      doc["value"] = wifiRSSI;
    }
    doc["entity"] = apSSID;
    doc["sensorId"] = chanName;
    doc["timestamp"] = dtimestamp;
    if(_realto_en) dtopic = _mqtt_prefix + "sys/" + chanName;
    else dtopic = "sys/devices/" + String(apSSID) + "/" + chanName;
    String jsonOutput;
    serializeJson(doc, jsonOutput);
    if(_mqtt_en){
      if(sinceLastUpload >= _upload_throttle){
       pubMqtt(dtopic, jsonOutput, true);
      }
    }
  }
}
