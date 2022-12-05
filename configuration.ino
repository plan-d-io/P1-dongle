boolean restoreConfig() {
  wifi_ssid = preferences.getString("WIFI_SSID");
  wifi_password = preferences.getString("WIFI_PASSWD");  
  wifiSTA = preferences.getBool("WIFI_STA");
  mqtt_en = preferences.getBool("MQTT_EN");
  mqtt_tls = preferences.getBool("MQTT_TLS");
  mqtt_host = preferences.getString("MQTT_HOST");
  mqtt_id = preferences.getString("MQTT_ID");
  mqtt_auth = preferences.getBool("MQTT_AUTH");
  mqtt_user = preferences.getString("MQTT_USER");
  mqtt_pass = preferences.getString("MQTT_PASS");
  mqtt_port = preferences.getUInt("MQTT_PORT");
  upload_throttle = preferences.getULong("UPL_THROTTLE");
  update_auto = preferences.getBool("UPD_AUTO");
  update_autoCheck = preferences.getBool("UPD_AUTOCHK");
  beta_fleet = preferences.getBool("BETA_FLT");
  fw_new = preferences.getUInt("FW_NEW");
  update_start = preferences.getBool("UPD_START");
  update_finish = preferences.getBool("UPD_FINISH");
  eid_en = preferences.getBool("EID_EN");
  eid_webhook = preferences.getString("EID_HOOK");
  ha_en = preferences.getBool("HA_EN");
  counter = preferences.getUInt("counter", 0);
  bootcount = preferences.getUInt("reboots", 0);
  last_reset = preferences.getString("LAST_RESET");
  dsmrVersion = preferences.getUInt("DM_DSMRV");
  trigger_interval = preferences.getUInt("TRG_INT");
  trigger_type = preferences.getUInt("TRG_TYPE");
  pls_en = preferences.getBool("PLS_EN");
  pls_mind1 = preferences.getInt("PLS_MIND1");
  pls_mind2 = preferences.getInt("PLS_MIND2");
  pls_multi1 = preferences.getInt("PLS_MULTI1");
  pls_multi2 = preferences.getInt("PLS_MULTI2");
  pls_type1 = preferences.getInt("PLS_TYPE1");
  pls_type2 = preferences.getInt("PLS_TYPE2");
  pls_unit1 = preferences.getString("PLS_UNT1");
  pls_unit2 = preferences.getString("PLS_UNT2");
  pls_off1 = preferences.getUInt("PLS_OFF1");
  pls_off2 = preferences.getUInt("PLS_OFF2");
  if(preferences.getBool("DM_ACTTAR") == true) dmActiveTariff = "1";
  else dmActiveTariff = "0";
  if(preferences.getBool("DM_VOLT1") == true) dmVoltagel1 = "1";
  else dmVoltagel1 = "0";
  if(preferences.getBool("DM_VOLT2") == true) dmVoltagel2 = "1";
  else dmVoltagel2 = "0";
  if(preferences.getBool("DM_VOLT3") == true) dmVoltagel3 = "1";
  else dmVoltagel3 = "0";
  if(preferences.getBool("DM_CUR1") == true) dmCurrentl1 = "1";
  else dmCurrentl1 = "0";
  if(preferences.getBool("DM_CUR2") == true) dmCurrentl2 = "1";
  else dmCurrentl2 = "0";
  if(preferences.getBool("DM_CUR3") == true) dmCurrentl3 = "1";
  else dmCurrentl3 = "0";
  if(preferences.getBool("DM_GAS") == true) dmGas = "1";
  else dmGas = "0";
  if(preferences.getBool("DM_TXT") == true) dmText = "1";
  else dmText = "0";
  configMeter();
  if(mqtt_en) mqttSave = true;
  if(eid_en) eidSave = true;
  if(ha_en) haSave = true;
  return true;
}

boolean saveConfig() {
  preferences.putString("WIFI_SSID", wifi_ssid);
  preferences.putString("WIFI_PASSWD", wifi_password);
  if(wifiSave) preferences.putBool("WIFI_STA", true);
  else preferences.putBool("WIFI_STA", wifiSTA);
  preferences.putBool("MQTT_EN", mqttSave);
  preferences.putBool("MQTT_TLS", mqtt_tls); 
  preferences.putString("MQTT_HOST", mqtt_host);
  preferences.putString("MQTT_ID", mqtt_id);
  preferences.putBool("MQTT_AUTH", mqtt_auth); 
  preferences.putString("MQTT_USER", mqtt_user);
  preferences.putString("MQTT_PASS", mqtt_pass);
  preferences.putUInt("MQTT_PORT", mqtt_port);
  preferences.putULong("UPL_THROTTLE", upload_throttle);
  preferences.putBool("UPD_AUTO", update_auto);
  preferences.putBool("UPD_AUTOCHK", update_autoCheck);
  preferences.putBool("BETA_FLT", beta_fleet);
  preferences.putUInt("FW_NEW", onlineVersion);
  preferences.putBool("UPD_START", update_start);
  preferences.putBool("UPD_FINISH", update_finish);
  preferences.putUInt("counter", counter);
  preferences.putUInt("reboots", bootcount);
  preferences.putBool("EID_EN", eidSave);
  preferences.putString("EID_HOOK", eid_webhook);
  preferences.putString("LAST_RESET", last_reset);
  preferences.putBool("HA_EN", haSave);
  preferences.putUInt("DM_DSMRV", dsmrVersion);
  preferences.putUInt("TRG_INT", trigger_interval);
  preferences.putUInt("TRG_TYPE", trigger_type);
  if (dmActiveTariff == "1") preferences.putBool("DM_ACTTAR", true);
  else preferences.putBool("DM_ACTTAR", false);
  if (dmVoltagel1 == "1") preferences.putBool("DM_VOLT1", true);
  else preferences.putBool("DM_VOLT1", false);
  if (dmVoltagel2 == "1") preferences.putBool("DM_VOLT2", true);
  else preferences.putBool("DM_VOLT2", false);
  if (dmVoltagel3 == "1") preferences.putBool("DM_VOLT3", true);
  else preferences.putBool("DM_VOLT3", false);
  if (dmCurrentl1 == "1") preferences.putBool("DM_CUR1", true);
  else preferences.putBool("DM_CUR1", false);
  if (dmCurrentl2 == "1") preferences.putBool("DM_CUR2", true);
  else preferences.putBool("DM_CUR2", false);
  if (dmCurrentl3 == "1") preferences.putBool("DM_CUR3", true);
  else preferences.putBool("DM_CUR3", false);
  if (dmGas == "1") preferences.putBool("DM_GAS", true);
  else preferences.putBool("DM_GAS", false);
  if (dmText == "1") preferences.putBool("DM_TXT", true);
  else preferences.putBool("DM_TXT", false);
  return true;
}

boolean saveBoots(){
  preferences.putUInt("reboots", bootcount);
  return true;
}

boolean resetConfig() {
  if(resetAll || resetWifi){
    Serial.print("Executing config reset");
    Serial.print("Executing wifi reset");
    preferences.remove("WIFI_SSID");
    preferences.remove("WIFI_PASSWD");
    preferences.remove("MQTT_HOST");
    preferences.putBool("WIFI_STA", false);
    preferences.putBool("UPD_AUTO", true);
    preferences.putBool("UPD_AUTOCHK", true);
    preferences.putString("LAST_RESET", "Restarting for config reset");
    preferences.end();
    syslog("Restarting for config reset", 2);
    delay(500);
    ESP.restart();
    return true;
  }
}

boolean initConfig() {
  String tempMQTT = preferences.getString("MQTT_HOST");
  if(tempMQTT == ""){
    preferences.putBool("MQTT_EN", true);
    mqtt_en = true;
    preferences.putString("MQTT_HOST", "realto.s2.eu.hivemq.cloud");
    mqtt_host = "realto.s2.eu.hivemq.cloud";
    preferences.putUInt("MQTT_PORT", 8883);
    mqtt_port = 8883;
    preferences.putBool("MQTT_TLS", true);
    mqtt_tls = true;
    preferences.putBool("MQTT_AUTH", true);
    mqtt_auth = true;
    preferences.putString("MQTT_USER", "realto-mqtt-client");
    mqtt_user = "realto-mqtt-client";
    preferences.putString("MQTT_PASS", "FCACq.w_CwLgZgxg_9oz");
    mqtt_pass = "FCACq.w_CwLgZgxg_9oz";
    preferences.putULong("UPL_THROTTLE", 60);
    upload_throttle = 60;
    tempMQTT = preferences.getString("MQTT_ID");
    if(tempMQTT == ""){
      preferences.putString("MQTT_ID", apSSID);
      mqtt_id = apSSID;
      preferences.putBool("HA_EN", false);
      ha_en = false;
    }
    preferences.putBool("UPD_AUTO", true); 
    preferences.putBool("UPD_AUTOCHK", true);
    preferences.putBool("BETA_FLT", true);
    preferences.putUInt("DM_DSMRV", 0);
    preferences.putBool("DM_ACTTAR", true);
    preferences.putBool("DM_VOLT1", true);
    preferences.putBool("DM_VOLT2", false);
    preferences.putBool("DM_VOLT3", false);
    preferences.putBool("DM_CUR1", false);
    preferences.putBool("DM_CUR2", false);
    preferences.putBool("DM_CUR3", false);
    preferences.putBool("DM_GAS", true);
    preferences.putBool("DM_TXT", false);
    preferences.putInt("TRG_INT", 10);
    preferences.putInt("TRG_TYPE", 0);
    preferences.putBool("PLS_EN", false);
    preferences.putInt("PLS_MIND1", 20);
    preferences.putInt("PLS_MIND2", 20);
    preferences.putInt("PLS_MULTI1", 1000);
    preferences.putInt("PLS_MULTI2", 1000);
    preferences.putInt("PLS_TYPE1", 0);
    preferences.putInt("PLS_TYPE2", 1);
    preferences.putString("PLS_UNT1", "kWh");
    preferences.putString("PLS_UNT2", "m³");
  }
  unsigned int tempMQTTint = preferences.getUInt("MQTT_PORT");
  if(tempMQTTint == 0){
    preferences.putUInt("MQTT_PORT", 1883);
    mqtt_port = 1883;
  }
  return true;
}
