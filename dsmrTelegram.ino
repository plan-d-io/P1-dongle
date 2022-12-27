void splitTelegram(String rawTelegram){
  sinceMeterCheck = 0;
  mTimeFound = false;
  meterError = true;
  int delimStart = 0;
  int delimEnd = 0;
  int eof = rawTelegram.lastIndexOf('\n');
  //Serial.println(rawTelegram);
  while(delimEnd < eof){
    delimEnd = rawTelegram.indexOf('\n', delimStart);
    String s = rawTelegram.substring(delimStart, delimEnd);
    delimStart = delimEnd+1;
    byte starts = s.indexOf('(');
    String key = s.substring(0, starts);
    if(key == "0-0:1.0.0"){
      byte ends = s.indexOf(')');  
      String sub = s.substring(starts+1, ends);
      int yeari = 2000 + sub.substring(0, 2).toInt();
      int monthi = sub.substring(2, 4).toInt();
      int dayi = sub.substring(4, 6).toInt();
      int houri = sub.substring(6, 8).toInt();
      int minutei = sub.substring(8, 10).toInt();
      int secondi = sub.substring(10, 12).toInt();
      String DST = sub.substring(12);
      dm_time.tm_sec = secondi;
      dm_time.tm_hour = houri;
      dm_time.tm_min = minutei;
      dm_time.tm_mday = dayi;
      dm_time.tm_mon = monthi - 1;      // months start from 0, so deduct 1
      dm_time.tm_year = yeari - 1900; // years since 1900, so deduct 1900
      dm_timestamp =  mktime(&dm_time);
      if(DST == "S"){
        dm_timestamp = dm_timestamp-(2*3600);
      }
      else{
        dm_timestamp = dm_timestamp-(3600);
      }
      mTimeFound = true;
      if(!wifiSTA) unitState = 2;
      else unitState = 4;
      meterError = false;
    }
    else{
      //Serial.println(key);
      for(int i = 0; i < sizeof(dsmrKeys) / sizeof(dsmrKeys[0]); i++){
        if(key == dsmrKeys[i][0]){
          if(dsmrKeys[i][1] == "0"){
            byte ends = s.indexOf(')');  
            String sub = s.substring(starts+1, ends);
          }
          else if(dsmrKeys[i][1] == "1"){
            byte ends = s.indexOf(')');  
            String sub = s.substring(starts+1, ends);
            int value = sub.toInt();
            processMeterValue(i, value, 0, false, "", dm_timestamp);
          }
          else if(dsmrKeys[i][1] == "2"){
            byte ends = s.indexOf(')');  
            String sub = s.substring(starts+1, ends);
            float value = sub.toFloat();
            processMeterValue(i, 0, value, true, "", dm_timestamp);
          }
          else if(dsmrKeys[i][1] == "3"){
            byte ends = s.indexOf(')');  
            String sub = s.substring(starts+1, ends);
            byte units = sub.indexOf('*');
            String valuetext = sub.substring(0, units);
            int value = valuetext.toInt();
            String unit = sub.substring(units+1);
            processMeterValue(i, value, 0, false, unit, dm_timestamp);
          }
          else if(dsmrKeys[i][1] == "4"){
            byte ends = s.indexOf(')');  
            String sub = s.substring(starts+1, ends);
            byte units = sub.indexOf('*');
            String valuetext = sub.substring(0, units);
            float value = valuetext.toFloat();
            String unit = sub.substring(units+1);
            processMeterValue(i, 0, value, true, unit, dm_timestamp);
          }
          else if(dsmrKeys[i][1] == "5"){
            byte ends = s.indexOf(')');  
            String sub = s.substring(starts+1, ends);
            int yeari = 2000 + sub.substring(0, 2).toInt();
            int monthi = sub.substring(2, 4).toInt();
            int dayi = sub.substring(4, 6).toInt();
            int houri = sub.substring(6, 8).toInt();
            int minutei = sub.substring(8, 10).toInt();
            int secondi = sub.substring(10, 12).toInt();
            String DST = sub.substring(12);
            mb1_time.tm_sec = secondi;
            mb1_time.tm_hour = houri;
            mb1_time.tm_min = minutei;
            mb1_time.tm_mday = dayi;
            mb1_time.tm_mon = monthi;      // months start from 0, so deduct 1
            mb1_time.tm_year = yeari - 1900; // years since 1970, so deduct 1970
            mb1_timestamp =  mktime(&mb1_time);
            if(DST == "S"){
              mb1_timestamp = mb1_timestamp-(2*3600);
            }
            else{
              mb1_timestamp = mb1_timestamp-(3600);
            }
            sub = s.substring(ends+2);
            ends = sub.indexOf(')');
            sub = sub.substring(0, ends);
            byte units = sub.indexOf('*');
            String valuetext = sub.substring(0, units);
            float value = valuetext.toFloat();
            String unit = sub.substring(units+1);
            if(unit == "m3") unit = "m³";
            processMeterValue(i, 0, value, true, unit, mb1_timestamp);
          }
        }
      } 
    }
  }
  if(!meterError) sumMeterTotals();
}

void processMeterValue(int dsmrKey, int imeasurement, float fmeasurement, boolean floatValue, String unit, unsigned long meterTime){
  if(dsmrKeys[dsmrKey][0] == "1-0:1.8.1") totConDay = fmeasurement;
  else if(dsmrKeys[dsmrKey][0] == "1-0:1.8.2") totConNight = fmeasurement;
  else if(dsmrKeys[dsmrKey][0] == "1-0:2.8.1") totInDay = fmeasurement;
  else if(dsmrKeys[dsmrKey][0] == "1-0:2.8.2") totInNight = fmeasurement;
  else if(dsmrKeys[dsmrKey][0] == "1-0:1.7.0") totPowCon = fmeasurement;
  else if(dsmrKeys[dsmrKey][0] == "1-0:2.7.0") totPowIn = fmeasurement;
  else if(dsmrKeys[dsmrKey][0] == "1-0:1.4.0") totPowIn = fmeasurement;
  else if(dsmrKeys[dsmrKey][0] == "1-0:32.7.0") volt1 = fmeasurement;
  else if(dsmrKeys[dsmrKey][0] == "1-0:52.7.0") volt2 = fmeasurement;
  else if(dsmrKeys[dsmrKey][0] == "1-0:72.7.0") volt3 = fmeasurement;
  DynamicJsonDocument doc(1024);
  doc["entity"] = "utility_meter";
  doc["sensorId"] = "utility_meter." + dsmrKeys[dsmrKey][3].substring(dsmrKeys[dsmrKey][3].lastIndexOf('/')+1);
  String friendly_name = String(dsmrKeys[dsmrKey][2]);
  friendly_name.toLowerCase();
  friendly_name = "Utility meter " + friendly_name;
  doc["friendly_name"] = friendly_name;
  if(dsmrKeys[dsmrKey][4] != "") doc["metric"] = dsmrKeys[dsmrKey][4];
  doc["metricKind"] = dsmrKeys[dsmrKey][5];
  if(floatValue) doc["value"] = fmeasurement;
  else doc["value"] = imeasurement;
  if(unit != "") doc["unit"] = unit;
  doc["timestamp"] = meterTime;
  String jsonOutput;
  serializeJson(doc, jsonOutput);
  if(mqtt_en && meterConfig[dsmrKey] == "1"){
    if(sinceLastUpload >= (upload_throttle * 1000)){
     pubMqtt(dsmrKeys[dsmrKey][3], jsonOutput, false);
    }
  }
}

void sumMeterTotals(){
  totCon = (totConDay + totConNight)*1.0;
  totConToday = totCon - totConYesterday;
  totIn = (totInDay + totInNight)*1.0;
  netPowCon = (totPowCon - totPowIn)*1.0;
  gasConToday = totGasCon - gasConYesterday;
  if(dm_time.tm_mday != prevDay){
    totConYesterday = totCon;
    gasConYesterday = totGasCon;
    prevDay = dm_time.tm_mday ;
  }
  if(mqtt_en){
    for(int i = 0; i < 3; i++){
      String totalsTopic = "";
      DynamicJsonDocument doc(1024);
      doc["entity"] = "utility_meter";
      doc["metric"] = "GridElectricityImport";
      doc["metricKind"] = "cumulative";
      if(i == 0){
        totalsTopic = "total_energy_consumed";
        doc["friendly_name"] = "Utility meter total energy consumed";
        doc["value"] = totCon;
        doc["unit"] = "kWh";
      }
      else if(i == 1){
        totalsTopic = "total_energy_injected";
        doc["metric"] = "GridElectricityExport";
        doc["friendly_name"] = "Utility meter total energy injected";
        doc["value"] = totIn;
        doc["unit"] = "kWh";
      }
      else{
        totalsTopic = "total_active_power";
        doc["metric"] = "GridElectricityPower";
        doc["friendly_name"] = "Utility meter total active power";
        doc["value"] = netPowCon;
        doc["metricKind"] = "gauge";
        doc["unit"] = "kW";
      }
      doc["timestamp"] = dm_timestamp;
      doc["sensorId"] = "utility_meter." + totalsTopic;
      totalsTopic = "data/devices/utility_meter/" + totalsTopic;
      String jsonOutput;
      serializeJson(doc, jsonOutput);
      if(sinceLastUpload >= (upload_throttle*1000)){
        pubMqtt(totalsTopic, jsonOutput, false);
      }
    }
    if(sinceLastUpload >= (upload_throttle*1000)){
      sinceLastUpload = 0;
    }
  }
}
