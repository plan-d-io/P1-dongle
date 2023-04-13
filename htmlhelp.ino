String getConfig() {
  StaticJsonDocument<840> configData;
  configData["innerHTML"]["HostValue"]["value"] = apSSID;
  configData["innerHTML"]["state_value"]["value"] = apSSID;
  configData["innerValues"]["wifi_ssid"]["value"] = wifi_ssid;
  configData["innerValues"]["mqtt_host"]["value"] = mqtt_host;
  configData["innerValues"]["mqtt_id"]["value"]  = mqtt_id;
  configData["innerValues"]["mqtt_user"]["value"]  = mqtt_user;
  configData["innerValues"]["mqtt_port"]["value"]  = mqtt_port;
  configData["innerValues"]["upload_throttle"]["value"]  = upload_throttle;
  configData["innerValues"]["eid_webhook"]["value"]  = eid_webhook;
  configData["innerSrc"]["mqtt_user"]["value"]  = apSSID;
  configData["innerTitles"]["mqtt_user"]["value"]  = apSSID;
  configData["innerChecks"]["mqtt_en"]["value"] = mqttSave;
  configData["innerChecks"]["mqtt_tls"]["value"] = mqtt_tls;
  configData["innerChecks"]["mqtt_auth"]["value"] = mqtt_auth;
  configData["innerChecks"]["eid_en"]["value"] = eidSave;
  configData["innerChecks"]["ha_en"]["value"] = haSave;
  String output;
  serializeJson(configData, output);
  return output;
}

String getIndexData() {
  StaticJsonDocument<840> indexData;
  String hostValue = "N/A";
  if(abs(netPowCon) < 1.0){
    if(mTimeFound) hostValue = String(netPowCon*1000.0) + " W";
    indexData["innerHTML"]["netPowCon"]["value"] = hostValue;
  }
  else{
    if(mTimeFound) hostValue = String(netPowCon) + " kW";
    indexData["innerHTML"]["netPowCon"]["value"] = hostValue;
  }
  if(mTimeFound) hostValue = String(totConToday) + " kWh";
  indexData["innerHTML"]["totConToday"]["value"] = hostValue;
  if(mTimeFound) hostValue = String(totCon) + " kWh";
  indexData["innerHTML"]["totCon"]["value"] = hostValue;
  if(mTimeFound) hostValue = String(totIn) + " kWh";
  indexData["innerHTML"]["totIn"]["value"] = hostValue;
  if(mTimeFound) hostValue = String(totGasCon) + " m³";
  indexData["innerHTML"]["totGasCon"]["value"] = hostValue;
  if(mTimeFound) hostValue = String(avgDem) + " kW";
  indexData["innerHTML"]["dmAvDem"]["value"] = hostValue;
  if(mTimeFound) hostValue = String(maxDemM) + " kW";
  indexData["innerHTML"]["dmMaxDemM"]["value"] = hostValue;
  String output;
  serializeJson(indexData, output);
  return output;
}

String getIndexStatic() {
  StaticJsonDocument<840> indexStatic;
  indexStatic["innerHTML"]["HostValue"]["value"] = apSSID; 
  indexStatic["innerHTML"]["footer"]["value"] = "Digital meter dongle V" + String(fw_ver/100.0) + " by plan-d.io"; 
  if(wifiSTA) indexStatic["innerTitles"]["WifiStrength"]["value"] = "Wifi connected";
  else indexStatic["innerTitles"]["WifiStrength"]["value"] = "Wifi not connected/configured";
  if(mqtt_en){
    if(wifiSTA && !mqttHostError && !mqttClientError && !httpsError) indexStatic["innerTitles"]["Cloud"]["value"] = "Cloud connected";
    else indexStatic["innerTitles"]["Cloud"]["value"] = "Cloud not connected";
  }
  else indexStatic["innerTitles"]["Cloud"]["value"] = "Cloud not configured";  
  if(mTimeFound) indexStatic["innerTitles"]["DigitalMeter"]["value"] = "Digital meter connected";
  else if (mTimeFound && meterError) indexStatic["innerTitles"]["DigitalMeter"]["value"] = "Unsupported digital meter connected";
  else indexStatic["innerTitles"]["DigitalMeter"]["value"] = "No digital meter connected";
  indexStatic["innerTitles"]["ExternalIO"]["value"] = "No external IO connected";
  String output;
  serializeJson(indexStatic, output);
  return output;
}

String getUnit() {
  StaticJsonDocument<256> unitData;
  unitData["innerChecks"]["update_auto"]["value"] = update_auto;
  unitData["innerChecks"]["update_autoCheck"]["value"] = update_autoCheck;
  unitData["innerChecks"]["alpha_fleet"]["value"] = alpha_fleet;
  unitData["innerChecks"]["beta_fleet"]["value"] = dev_fleet;
  String output;
  serializeJson(unitData, output);
  return output;
}

String getDm() {
  //Note to self: for the love of God please refactor this
  StaticJsonDocument<840> dmData;
  dmData["innerChecks"]["dmActPow"]["value"] = true;
  dmData["innerChecks"]["dmTotCon"]["value"] = true;
  dmData["innerChecks"]["dmTotIn"]["value"] = true;
  if(dmAvDem == "1") dmData["innerChecks"]["dmAvDem"]["value"] = true;
  else dmData["innerChecks"]["dmAvDem"]["value"] = false;
  if(dmMaxDemM == "1") dmData["innerChecks"]["dmMaxDemM"]["value"] = true;
  else dmData["innerChecks"]["dmMaxDemM"]["value"] = false;
  if(dsmrVersion == 0) dmData["innerValues"]["dsmrVersion"]["value"] = "DSMR P1 V5.0.2 (Fluvius BE)";
  if(dmPowCon == "1") dmData["innerChecks"]["dmPowCon"]["value"] = true;
  else dmData["innerChecks"]["dmPowCon"]["value"] = false;
  if(dmPowIn == "1") dmData["innerChecks"]["dmPowIn"]["value"] = true;
  else dmData["innerChecks"]["dmPowIn"]["value"] = false;
  if(dmTotCont1 == "1") dmData["innerChecks"]["dmTotCon1"]["value"] = true;
  else dmData["innerChecks"]["dmTotCon1"]["value"] = false;
  if(dmTotCont2 == "1") dmData["innerChecks"]["dmTotCon2"]["value"] = true;
  else dmData["innerChecks"]["dmTotCon2"]["value"] = false;
  if(dmTotInt1 == "1") dmData["innerChecks"]["dmTotInt1"]["value"] = true;
  else dmData["innerChecks"]["dmTotInt1"]["value"] = false;
  if(dmTotInt2 == "1") dmData["innerChecks"]["dmTotInt2"]["value"] = true;
  else dmData["innerChecks"]["dmTotInt2"]["value"] = false;
  if(dmActiveTariff == "1") dmData["innerChecks"]["dmActiveTariff"]["value"] = true;
  else dmData["innerChecks"]["dmActiveTariff"]["value"] = false;
  if(dmVoltagel1 == "1") dmData["innerChecks"]["dmVoltagel1"]["value"] = true;
  else dmData["innerChecks"]["dmVoltagel1"]["value"] = false;
  if(dmVoltagel2 == "1") dmData["innerChecks"]["dmVoltagel2"]["value"] = true;
  else dmData["innerChecks"]["dmVoltagel2"]["value"] = false;
  if(dmVoltagel3 == "1") dmData["innerChecks"]["dmVoltagel3"]["value"] = true;
  else dmData["innerChecks"]["dmVoltagel3"]["value"] = false;
  if(dmCurrentl1 == "1") dmData["innerChecks"]["dmCurrentl1"]["value"] = true;
  else dmData["innerChecks"]["dmCurrentl1"]["value"] = false;
  if(dmCurrentl2 == "1") dmData["innerChecks"]["dmCurrentl2"]["value"] = true;
  else dmData["innerChecks"]["dmCurrentl2"]["value"] = false;
  if(dmCurrentl3 == "1") dmData["innerChecks"]["dmCurrentl3"]["value"] = true;
  else dmData["innerChecks"]["dmCurrentl3"]["value"] = false;
  if(dmGas == "1") dmData["innerChecks"]["dmGas"]["value"] = true;
  else dmData["innerChecks"]["dmGas"]["value"] = false;
  if(dmText == "1") dmData["innerChecks"]["dmText"]["value"] = true;
  else dmData["innerChecks"]["dmText"]["value"] = false;
  dmData["innerValues"]["trigger_interval"]["value"]  = trigger_interval;
  String output;
  serializeJson(dmData, output);
  return output;
}

String getIo() {
  StaticJsonDocument<256> ioData;
  ioData["innerChecks"]["pls_en"]["value"] = pls_en;
  ioData["innerValues"]["pls_type1"]["value"] = pls_type1;
  ioData["innerValues"]["pls_multi1"]["value"] = pls_multi1;
  ioData["innerValues"]["pls_unit1"]["value"] = pls_unit1;
  ioData["innerValues"]["pls_mind1"]["value"] = pls_mind1;
  ioData["innerValues"]["pls_type2"]["value"] = pls_type2;
  ioData["innerValues"]["pls_multi2"]["value"] = pls_multi2;
  ioData["innerValues"]["pls_unit2"]["value"] = pls_unit2;
  ioData["innerValues"]["pls_mind2"]["value"] = pls_mind2;
  String output;
  serializeJson(ioData, output);
  return output;
}
