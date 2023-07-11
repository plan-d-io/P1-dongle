/*Process the meter telegram and configure its options*/

static const keyConfig dsmrKeys[] PROGMEM = {
  /*The list of all possible DSMR keys and how to parse them.
   * { "DSMR key code", key type, "HA device type", "user-readable name", "mqtt topic", retain }
   * Key types:
   *  0: other
   *  1: numeric without unit, e.g. 0-0:96.14.0(0002)
   *  2: numeric with unit, e.g. 1-0:1.4.0(02.351*kW)
   *  3: timestamped (Mbus) message, e.g. 0-1:24.2.3(200512134558S)(00112.384*m3)
   *  4: string, e.g. 0-0:96.13.0(3031323334353)
   *  The key type determines how the associated value will be parsed.
   * Home Assistant device types: see https://developers.home-assistant.io/docs/core/entity/sensor/#available-device-classes
   *  Examples: "" (none), power, energy, voltage, current, natural gas, water, heat.
   *  The device type determines how the value will be registered and represented in Home Assistant. You can leave this to "" (none) if you don't use HA.
   * Mqtt topic:
   *  The MQTT topic to post the measurement to. Will be prefixed by mqtt_prefix (standard: data/devices/utility_meter/).
   *  If left empty, the firmware will use the user readable name cast to lowercase with spaces replaced by _
   * Retain:
   *  If the MQTT payload should be sent with the retain flag.
   * IMPORTANT
   *  DSMR keys are adressed in the firmware by their order in this array. DO NOT rearrange keys.
   *  If you want to add new keys, add them to the bottom of the array.
   */
    /*The first three keys are custom keys, required for dongle operation*/
    { "A-0:0.0.1", 2, "energy", "Total energy consumed",  "",  false},
    { "A-0:0.0.2", 2, "energy", "Total energy injected",  "",  false},
    { "A-0:0.0.3", 2, "power", "Total active power",  "",  false},
    /*Standard keys*/
    { "1-0:1.8.1", 2, "energy", "Total consumption T1",  "",  false},
    { "1-0:1.8.2", 2, "energy", "Total consumption T2",  "",  false},
    { "1-0:2.8.1", 2, "energy", "Total injection T1",  "",  false},
    { "1-0:2.8.2", 2, "energy", "Total injection T2",  "",  false},
    { "0-0:96.14.0", 1, "", "Active tariff period",  "",  false},
    { "1-0:1.7.0", 2, "power", "Active power consumption",  "",  false},
    { "1-0:2.7.0", 2, "power", "Active power injection",  "",  false},
    { "1-0:1.4.0", 2, "power", "Current average demand",  "",  false},
    { "1-0:1.6.0", 3, "power", "Maximum demand current month",  "",  false},
    { "1-0:32.7.0", 2, "voltage", "Voltage phase 1", "",  false},
    { "1-0:52.7.0", 2, "voltage", "Voltage phase 2", "",  false},
    { "1-0:72.7.0", 2, "voltage", "Voltage phase 3", "",  false},
    { "1-0:31.7.0", 2, "current", "Current phase 1", "",  false},
    { "1-0:51.7.0", 2, "current", "Current phase 2", "",  false},
    { "1-0:71.7.0", 2, "current", "Current phase 3", "",  false},
    { "0-0:96.13.0", 4, "", "Text message", "",  false}
};

static const mbusConfig mbusKeys[] PROGMEM = {
  /*The availability and id of M-bus meters (water, gas, heat, ...) is dependent on your type of installation, so you can't set the Mbus OBIS keys to be extracted
   * from the telegram beforehand.
   * The dongle will automatically detect the type and id of connected M-bus meter and process the data according to the fields in this list (e.g. MQTT topic).
   * If the M-bus meter reports both temperature (base) and non-temperature corrected values, the temperature corrected value will be used.
   * If you want to extract a specific M-bus key, add it to the dsmrKeys list above.
   * Format: { Mbus meter type, HA device type, "user-readable name", "mqtt topic", retain }
   * Mqtt topic: see note in dsmrKeys above
   * Mbus meter types:
   *  3: natural gas
   *  4: heat/cold
   *  7: water
   */
    { 3, "gas", "Natural gas consumption", "",  false},
    { 4, "", "Heat", "",  false},
    { 7, "water", "Water consumption", "",  false}
};

void processMeterTelegram(String rawTelegram){
  /*The meter telegram consists of multiple lines with a key(value) pair, e.g. 1-0:2.7.0(01.100*kW) etc.
  /* Every key starts on a newline. First we split up the meter telegram per line into its key(value) pairs.*/
  int delimStart = 0;
  int delimEnd = 0;
  int eof = rawTelegram.lastIndexOf('\n');
  if(extendedTelegramDebug) Serial.println(rawTelegram);
  if(_push_full_telegram) {}//something to push raw telegram here
  DynamicJsonDocument realtoReadings(1024);
  int pass = 0;
  while(delimEnd < eof){
    pass = pass + 1;
    delimEnd = rawTelegram.indexOf('\n', delimStart);       //Get the location of the newline char at the end of the line
    String s = rawTelegram.substring(delimStart, delimEnd); //Extract one line from the telegram
    delimStart = delimEnd+1;                                //Set the start of the next line (for the next iteration)
    /*Every line contains a key(value) pair, e.g. 1-0:2.7.0(01.100*kW)*/
    byte valueStart = s.indexOf('(');                       //The key is the part before the first bracket, e.g. 1-0:2.7.0
    String key = s.substring(0, valueStart);
    byte valueEnd = s.lastIndexOf(')');                     //The value is the part between the brackets, e.g. 01.100*kW
    String value = s.substring(valueStart+1, valueEnd);
    /*We now have every key(value) pair, so lets do something with them*/
    float splitValue;
    String splitUnit, splitString;
    struct tm splitTime;
    time_t splitTimestamp;
    if(telegramDebug){
      Serial.print(key);
      Serial.print(": ");
      Serial.print(value);
    }
    /* First, we extract the base keys we always need (hardcoded) and which should always be present in any type of DSMR telegram
     * These keys are needed for dongle operation (e.g. metertime) or to calculate values displayed in the webmin
     * We don't do any other processing, like MQTT push, in this part */
    /*Flemish DSMR*/
    if(key == "0-0:96.1.4"){
      if(telegramDebug){
        Serial.print(": Found Belgian DSMR version");
      }
    }
    /*Dutch DSMR*/
    if(key == "1-3:0.2.8"){ 
      if(telegramDebug){
        Serial.print(": Found Dutch DSMR version");
      }
    }
    /*Metertime*/
    if(key == "0-0:1.0.0"){
      splitMeterTime(value, splitTime, splitTimestamp);
      meterTimestamp = splitTimestamp;
      if(_realto_en) realtoReadings["timestamp"] = meterTimestamp;
      if(telegramDebug){
        Serial.print(": Meter time: ");
        Serial.print(splitTimestamp);
        char timeStringBuff[30];
        strftime(timeStringBuff, sizeof(timeStringBuff), " %Y/%m/%d %H:%M:%S", &splitTime);
        Serial.print(timeStringBuff);
      }
      if(!timeSet && !timeconfigured){ //If no NTP service configured (or resync required), use meter time to set internal clock
        Serial.println("");
        syslog("Syncing to to metertime", 0);
        localtime(&splitTimestamp);
        timeSet = true;
      }
      sinceMeterCheck = 0; //The metertime key is used to check for the presence of the digital meter
    }
    /*Consumed energy tariff 1*/
    if(key == "1-0:1.8.1"){
      splitWithUnit(value, splitValue, splitUnit);
      totConT1 = splitValue;
    }
    /*Consumed energy tariff 2*/
    if(key == "1-0:1.8.2"){      
      splitWithUnit(value, splitValue, splitUnit);
      totConT2 = splitValue;
    }
    /*Injected energy tariff 1*/
    if(key == "1-0:2.8.1"){      
      splitWithUnit(value, splitValue, splitUnit);
      totInT1 = splitValue;
    }
    /*Injected energy tariff 2*/
    if(key == "1-0:2.8.2"){    
      splitWithUnit(value, splitValue, splitUnit);
      totInT2 = splitValue;
    }
    /*Actual power consumption*/
    if(key == "1-0:1.7.0"){      
      splitWithUnit(value, splitValue, splitUnit);
      powCon = splitValue;
    }
    /*Actual power injection*/
    if(key == "1-0:2.7.0"){      
      splitWithUnit(value, splitValue, splitUnit);
      powIn = splitValue;
    }
    /*Process minimum required readings*/
    totCon = totConT1 + totConT2;
    totIn = totInT1 + totInT2;
    netPowCon = powCon - powIn;   
    /*Next, we check if the key is in the dsmrKeys[] list so we know if and how to process it*/
    for(int i = 0; i < sizeof(dsmrKeys)/sizeof(dsmrKeys[0]); i++){
      if(key == dsmrKeys[i].dsmrKey){
        if(dsmrKeys[i].keyType == 0 || dsmrKeys[i].keyType == 4){
          /*"Other" or "string" values (currently handled the same) just get their raw value forwarded, so no need to do any processing.
          Might change in the future.*/
        }
        else if(dsmrKeys[i].keyType == 1){
          /*Numeric value with no unit*/
          splitNoUnit(value, splitValue);
        }
        else if(dsmrKeys[i].keyType == 2){
          /*Numeric value with unit*/
          splitWithUnit(value, splitValue, splitUnit);
        }
        else if(dsmrKeys[i].keyType == 3){
          /*timestamped (Mbus) message*/
          splitWithTimeAndUnit(value, splitValue, splitUnit, splitTime, splitTimestamp);
        }
        else{
          //No need for any processing
        }  
        /*Check if the key,value pair needs to be pushed
         * The unsigned long _key_pushlist contains 32 bits indicating if the value needs to be pushed (1) or not (0)
         * E.g. if _key_pushlist is 329, its binary representation is 00000000000000000000000101001001
         * The LSB (at the rightmost side) represents dsmrKeys[0], the bit to the left of it dsmrKeys[1] etc.
         * We boolean AND the _key_pushlist with a binary mask 1, which is shifted to the left according to which key in dsmrKeys[] we are comparing
         * If the result is 1, this means the key,value pair must be pushed
         * E.g. if _key_pushlist is 329, dsmrKeys[0] must be pushed, dsmrKeys[1] not, dsmrKeys[2] not, dsmrKeys[3] must, etc.
         * Sounds complicated, but this way we only need one variable in NVS to store up to 32 options.
         */
        unsigned long mask = 1;
        mask <<= i;
        unsigned long test = _key_pushlist & mask;
        if(test > 0){
          if(telegramDebug){
            Serial.print(" - ");
            Serial.print(dsmrKeys[i].keyName);
            Serial.print(": ");
          }
          if(dsmrKeys[i].keyType == 0 || dsmrKeys[i].keyType == 4){
            /*Other or string value (currently handled the same)*/
            pushDSMRValue(key, 0, "", meterTimestamp, dsmrKeys[i].deviceType, dsmrKeys[i].keyName, dsmrKeys[i].keyTopic, value);
          }
          else if(dsmrKeys[i].keyType == 1){
            /*Numeric value with no unit*/
            pushDSMRValue(key, splitValue, "", meterTimestamp, dsmrKeys[i].deviceType, dsmrKeys[i].keyName, dsmrKeys[i].keyTopic, "");
          }
          else if(dsmrKeys[i].keyType == 2){
            /*Numeric value with unit*/
            pushDSMRValue(key, splitValue, splitUnit, meterTimestamp, dsmrKeys[i].deviceType, dsmrKeys[i].keyName, dsmrKeys[i].keyTopic, "");
          }
          else if(dsmrKeys[i].keyType == 3){
            /*timestamped (Mbus) message*/
            pushDSMRValue(key, splitValue, splitUnit, splitTimestamp, dsmrKeys[i].deviceType, dsmrKeys[i].keyName, dsmrKeys[i].keyTopic, "");
          }
          else{
            if(telegramDebug) Serial.print("undef");
          }
          if(telegramDebug){
            if(dsmrKeys[i].keyType == 0 || dsmrKeys[i].keyType == 4){
              Serial.print(value);
            }
            else{
              if(fmodf(splitValue, 1.0) == 0) Serial.print(int(splitValue));
              else Serial.print(round2(splitValue));
              Serial.print(" ");
              if(dsmrKeys[i].keyType == 2 || dsmrKeys[i].keyType == 3) Serial.print(splitUnit);
              if(dsmrKeys[i].keyType == 3){
                Serial.print(" (");
                Serial.print(splitTimestamp);
                Serial.print(")");
              }
            }
            String tempTopic = _mqtt_prefix;
            if(dsmrKeys[i].keyTopic == ""){ //Use key_name as mqtt topic if keyTopic left empty
              tempTopic += dsmrKeys[i].keyName;
              tempTopic.replace(" ", "_");
              tempTopic.toLowerCase();
            }
            else{
              tempTopic += dsmrKeys[i].keyTopic;
              tempTopic.replace(" ", "_");
              tempTopic.toLowerCase();
            }
            if(telegramDebug){
              Serial.print(", ");
              Serial.print(tempTopic);
            }
          }
          if(mqttPushCount == 2) haAutoDiscovery(key, splitUnit, dsmrKeys[i].deviceType, dsmrKeys[i].keyName, dsmrKeys[i].keyTopic);
        }
      }
      if(_realto_en){
        for(int j = 0; j < sizeof(realtoKeys)/sizeof(realtoKeys[0]); j++){
          if(key == realtoKeys[j]){
            if(fmodf(splitValue, 1.0) == 0) realtoReadings[key]["value"] = int(splitValue);
            else realtoReadings[key]["value"] = round2(splitValue);
            if(splitUnit != "") realtoReadings[key]["unit"] = splitUnit;
          }
        }
      }
    }
    /*Check if M-Bus meters are present. If so, register them into the mbusMeter array.
     * We only do this check during startup (first 2 telegrams), as M-bus meters do not change during operation
     */
    if(telegramCount < 2){
      registerMbusMeter(key, value);
    }
    else{
      /*Parse the Mbus OBIS key,value pairs according to the parameters set for each detected Mbus meter in the mbusMeter array */
      for(int i = 0; i < sizeof(mbusMeter)/sizeof(mbusMeter[0]); i++){
        if(key == mbusMeter[i].mbusKey){  //check if the key matches a key stored in the mbusMeter array
          for(int j = 0; j < sizeof(mbusKeys)/sizeof(mbusKeys[0]); j++){  //if so, process the key according the parameters set for that key
            if(mbusMeter[i].type == mbusKeys[j].keyType){
              splitWithTimeAndUnit(value, splitValue, splitUnit, splitTime, splitTimestamp);
              if(mbusMeter[i].enabled == true) {       
                if(mqttPushCount == 2) haAutoDiscovery(key, splitUnit, mbusKeys[j].deviceType, mbusKeys[j].keyName, mbusKeys[j].keyTopic);              
                pushDSMRValue(key, splitValue, splitUnit, splitTimestamp, mbusKeys[j].deviceType, mbusKeys[j].keyName, mbusKeys[j].keyTopic, "");
              }
              if(_realto_en){
                for(int k = 0; k < sizeof(realtoKeys)/sizeof(realtoKeys[0]); k++){
                  if(key == realtoKeys[k]){
                    if(fmodf(splitValue, 1.0) == 0) realtoReadings[key]["value"] = int(splitValue);
                    else realtoReadings[key]["value"] = round2(splitValue);
                    if(splitUnit != "") realtoReadings[key]["unit"] = splitUnit;
                  }
                }
              }
              if(telegramDebug){
                String tempTopic = _mqtt_prefix;
                if(mbusKeys[j].keyTopic == ""){
                  tempTopic += mbusKeys[j].keyName;
                  tempTopic.replace(" ", "_");
                  tempTopic.toLowerCase();
                }
                else{
                  tempTopic += mbusKeys[j].keyTopic;
                  tempTopic.replace(" ", "_");
                  tempTopic.toLowerCase();
                }
                Serial.print(" - ");
                Serial.print(mbusKeys[j].keyName);
                Serial.print(": ");
                if(fmodf(splitValue, 1.0) == 0) Serial.print(int(splitValue));
                else Serial.print(round2(splitValue));
                Serial.print(" ");
                Serial.print(splitUnit);
                Serial.print(" (");
                Serial.print(splitTimestamp);
                //Hier iets oplossen: timestamp klopt niet!
                Serial.print("), ");
                Serial.print(tempTopic);
              }
            }
          }
        }
      }
    }
    if(telegramDebug) Serial.println("");
  }
  if(_realto_en) serializeJson(realtoReadings, Serial);
  if(extendedTelegramDebug) Serial.println("finished telegram");
  telegramCount++;
  if(mqttPushCount < 2) haEraseDevice();
  if(mqttPushCount == 2){
    haAutoDiscovery("", "kWh", "energy", "Total energy consumed", "");
    haAutoDiscovery("", "kWh", "energy", "Total energy injected", "");
    haAutoDiscovery("", "kW", "power", "Total active power", "");
  }
  String key = "A-0:0.0.1";
  pushDSMRValue(key, totCon, "kWh", meterTimestamp, "energy", "Total energy consumed", "", "");
  key = "A-0:0.0.2";
  pushDSMRValue(key, totIn, "kWh", meterTimestamp, "energy", "Total energy injected", "", "");
  key = "A-0:0.0.3";
  pushDSMRValue(key, netPowCon, "kW", meterTimestamp, "power", "Total active power", "", "");
  if(mqttPushCount > 0) mqttPushCount++;
  
}

/*The following helper functions split the value part of each key,value pair according to the type of value*/
void splitToString(String &value, String &splitString){
  //Nothing to see here (yet)
}

void splitWithUnit(String &value, float &splitValue, String &splitUnit){
  /*Split a value containing a unit*/
  int unitStart = value.indexOf('*');
  splitValue = value.substring(0, unitStart).toFloat();
  splitUnit = value.substring(unitStart+1);
}

void splitNoUnit(String &value, float &splitValue){
  /*Split a value containing no unit*/
  splitValue = value.toFloat();
}

void splitMeterTime(String &value, struct tm &splitTime, time_t &splitTimestamp){
  /*Split a timestamp, returning a tm struct and a time_t object*/
  splitTime.tm_year = (2000 + value.substring(0, 2).toInt()) - 1900; //Time.h years since 1900, so deduct 1900
  splitTime.tm_mon = (value.substring(2, 4).toInt()) - 1; //Time.h months start from 0, so deduct 1
  splitTime.tm_mday = value.substring(4, 6).toInt();
  splitTime.tm_hour = value.substring(6, 8).toInt();
  splitTime.tm_min = value.substring(8, 10).toInt();
  splitTime.tm_sec = value.substring(10, 12).toInt();
  splitTimestamp =  mktime(&splitTime);
  /*Metertime is in local time. DST shows the difference in hours to UTC*/
  if(value.substring(12) == "S"){
    splitTime.tm_isdst = 1;
    splitTimestamp = splitTimestamp - (2*3600);
  }
  else if(value.substring(12) == "W"){
    splitTime.tm_isdst = 0;
    splitTimestamp = splitTimestamp - (1*3600);
  }
  else splitTime.tm_isdst = -1;
}

void splitWithTimeAndUnit(String &value, float &splitValue, String &splitUnit, struct tm &splitTime, time_t &splitTimestamp){
  /*Split a timestamped value containing a unit (e.g. Mbus value)*/
  String buf = value;
  int timeEnd = value.indexOf(')');
  int valueStart = value.indexOf('(')+1;
  value = buf.substring(0, timeEnd);
  splitMeterTime(value, splitTime, splitTimestamp);
  value = buf.substring(valueStart);
  splitWithUnit(value, splitValue, splitUnit);
}

void registerMbusMeter(String &key, String &value){
  /*The number and type of connected Mbus meters, as well as their Mbus ID and value type they report (standard or non-temperature compensated)
   * can only be determined during runtime by parsing the meter telegram. This function does just that. It scans the telegram for Mbus
   * OBIS codes. The detected meters and their parameters are then stored in the mbusMeter array.
   * During telegram processing, the mbusMeter array is cross referenced with the mbusConfig array to determine how the value should be pushed
   * to the data broker.
   */
  for(int i = 0; i < sizeof(mbusMeter)/sizeof(mbusMeter[0]); i++){
    String tempMbusKey = "0-" + String(i+1) + ":24.1.0";
    if(key == tempMbusKey){
      mbusMeter[i].type = value.toInt();
      /*Check if the Mbus key,value pair needs to be pushed.
       * This uses the same method as the regular DSMR keys, but with a 16-bit integer instead of a 32-bit long
       */
      if(mbusMeter[i].type >=0 && mbusMeter[i].type <= 16){
        unsigned int mask = 1;
        mask <<= mbusMeter[i].type;
        unsigned long test = _mbus_pushlist & mask;
        if(test > 0){
          mbusMeter[i].enabled = true;
        }
      }
      if(telegramCount == 1 && telegramDebug){
        Serial.print(" Found ");
        Serial.print(tempMbusKey);
        Serial.print(", meter of type ");
        Serial.print(mbusMeter[i].type);
        if(mbusMeter[i].type == 3) Serial.print(" (gas)");
        else if(mbusMeter[i].type == 7) Serial.print(" (water)");
        else Serial.print(" (other)");
        Serial.print(", registering");
        if (mbusMeter[i].enabled == true) Serial.print(" and enabling");
      }
    }
    /*Meter ID can be reported in regular (Dutch) or DIN43863-5 (Fluvius) format*/
    tempMbusKey = "0-" + String(i+1) + ":96.1.0";
    if(key == tempMbusKey){ //regular
      mbusMeter[i].id = value;
    }
    tempMbusKey = "0-" + String(i+1) + ":96.1.1";
    if(key == tempMbusKey){ //DIN43863-5
      mbusMeter[i].id = value;
    }
    /*Value can be reported in base or non-temperature compensated format. Priority to base*/
    tempMbusKey = "0-" + String(i+1) + ":24.2.3";
    if(key == tempMbusKey){ //non-temperature compensated
      if(mbusMeter[i].measurementType != 1){
        mbusMeter[i].measurementType = 3;
        mbusMeter[i].mbusKey = tempMbusKey;
      }
    }
    tempMbusKey = "0-" + String(i+1) + ":24.2.1";
    if(key == tempMbusKey){ //base
      mbusMeter[i].measurementType = 1;
      mbusMeter[i].mbusKey = tempMbusKey;
    }
  }
}

void pushDSMRValue(String &key, float measurementValue, String measurementUnit, time_t measurementTimestamp, String deviceType, String friendlyName, String mqttTopic, String rawKey){
  String tempTopic = _mqtt_prefix;
  String jsonOutput;
  bool pushSuccess = false;
  if(mqttTopic == ""){           //Use key_name as mqtt topic if keyTopic left empty
    tempTopic += friendlyName;
    tempTopic.replace(" ", "_");
    tempTopic.toLowerCase();
  }
  else{
    tempTopic += mqttTopic;
    tempTopic.replace(" ", "_");
    tempTopic.toLowerCase();
  }
  if(_payload_format > 0){ //minimal JSON payload format
    DynamicJsonDocument doc(1024);
    if(measurementValue == 0 && rawKey != ""){
      doc["value"] = rawKey;
    }
    else{
      if(fmodf(measurementValue, 1.0) == 0) doc["value"] = int(measurementValue);
      else doc["value"] = round2(measurementValue);
    }
    if(measurementUnit != "") doc["unit"] = measurementUnit;
    doc["timestamp"] = measurementTimestamp;
    if(_payload_format > 1){ //standard JSON payload format
      //friendlyName.toLowerCase();
      String sensorId = "Utility meter." + friendlyName;
      friendlyName = "Utility meter " + friendlyName;
      doc["friendly_name"] = friendlyName;
      sensorId.toLowerCase();
      sensorId.replace(" ", "_");
      doc["sensorId"] = sensorId;
      if(_payload_format > 2){ //COFY payload format
        doc["entity"] = "utility_meter";
        if(friendlyName == "Total energy consumed"){
          doc["metric"] = "GridElectricityImport";
          doc["metricKind"] = "cumulative";
        }
        else if(friendlyName == "Total energy injected"){
          doc["metric"] = "GridElectricityExport";
          doc["metricKind"] = "cumulative";
        }
        else if(friendlyName == "Total active power"){
          doc["metric"] = "GridElectricityPower";
          doc["metricKind"] = "gauge";
        }
        else{
          for(int k = 0; k < sizeof(cofyKeys)/sizeof(cofyKeys[0]); k++){
            if(key == cofyKeys[k][0]){
              if(cofyKeys[k][1] != "") doc["metric"] = cofyKeys[k][1];
              doc["metricKind"] = cofyKeys[k][2];
            }
          }
        }
      }
    }
    serializeJson(doc, jsonOutput);
    pushSuccess = pubMqtt(tempTopic, jsonOutput, false);
    if(mqttDebug && pushSuccess){
      Serial.println("");
      Serial.print(tempTopic);
      Serial.print(" ");
      serializeJson(doc, Serial);
    }
  }
  else{
    if(fmodf(measurementValue, 1.0) == 0){
      pushSuccess = pubMqtt(tempTopic, String(int(measurementValue)), false);
      if(mqttDebug && pushSuccess) Serial.println(int(measurementValue));
    }
    else{
      pushSuccess = pubMqtt(tempTopic, String(round2(measurementValue)), false);
      if(mqttDebug && pushSuccess) Serial.println(round2(measurementValue));
    }
    
  }
  if(pushSuccess && mqttPushCount == 0) mqttPushCount = 1;
}
