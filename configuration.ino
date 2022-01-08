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
  fw_new = preferences.getUInt("FW_NEW");
  update_start = preferences.getBool("UPD_START");
  update_finish = preferences.getBool("UPD_FINISH");
  eid_en = preferences.getBool("EID_EN");
  eid_webhook = preferences.getString("EID_HOOK");
  ha_en = preferences.getBool("HA_EN");
  counter = preferences.getUInt("counter", 0);
  bootcount = preferences.getUInt("reboots", 0);
  if(mqtt_en) mqttSave = true;
  if(eid_en) eidSave = true;
  if(ha_en) haSave = true;
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
  preferences.putUInt("FW_NEW", onlineVersion);
  preferences.putBool("UPD_START", update_start);
  preferences.putBool("UPD_FINISH", update_finish);
  preferences.putUInt("counter", counter);
  preferences.putUInt("reboots", bootcount);
  preferences.putBool("EID_EN", eidSave);
  preferences.putString("EID_HOOK", eid_webhook);
  preferences.putBool("HA_EN", haSave);
  return true;
}

boolean saveBoots(){
  preferences.putUInt("reboots", bootcount);
  Serial.println("Boots saved");
  return true;
}

boolean resetConfig() {
  if(resetAll){
    Serial.print("Executing config reset");
  }
  else if(resetWifi){
    Serial.print("Executing wifi reset");
    preferences.remove("WIFI_SSID");
    preferences.remove("WIFI_PASSWD");
    preferences.putBool("WIFI_STA", false);
    preferences.putBool("UPD_AUTO", true);
    preferences.putBool("UPD_AUTOCHK", true);
    preferences.end();
    ESP.restart();
  }
}

boolean initConfig() {
  String tempMQTT = preferences.getString("MQTT_HOST");
  if(tempMQTT == ""){
    preferences.putString("MQTT_HOST", "10.42.0.1");
    mqtt_host = "10.42.0.1";
    preferences.putUInt("MQTT_PORT", 1883);
    mqtt_port = 1883;
    tempMQTT = preferences.getString("MQTT_ID");
    if(tempMQTT == ""){
      preferences.putString("MQTT_ID", apSSID);
      mqtt_id = apSSID;
    }
    preferences.putBool("UPD_AUTO", true); 
    preferences.putBool("UPD_AUTOCHK", true);
  }
  unsigned int tempMQTTint = preferences.getUInt("MQTT_PORT");
  if(tempMQTTint == 0){
    preferences.putUInt("MQTT_PORT", 1883);
    mqtt_port = 1883;
  }
}
