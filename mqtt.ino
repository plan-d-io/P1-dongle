void setupMqtt() {
  String mqttinfo = "MQTT enabled! Will connect as " + mqtt_id;
  if (mqtt_auth) {
    mqttinfo = mqttinfo + " using authentication, with username " + mqtt_user;
  }
  syslog(mqttinfo, 0);
  if(mqtt_tls){
    mqttclientSecure.setClient(*client);
    if(upload_throttle > 0){
      mqttclientSecure.setKeepAlive(upload_throttle +1).setSocketTimeout(upload_throttle +1);
    }
  }
  else {
    mqttclient.setClient(wificlient);
    if(upload_throttle > 0){
      mqttclient.setKeepAlive(upload_throttle +1).setSocketTimeout(upload_throttle +1);
    }
  }
  /*Set broker location*/
  IPAddress addr;
  if (mqtt_host.length() > 0) {
    if (addr.fromString(mqtt_host)) {
      syslog("MQTT host has IP address " + mqtt_host, 0);
      if(mqtt_tls){
        mqttclientSecure.setServer(addr, mqtt_port);
        mqttclientSecure.setCallback(callback);
      }
      else{
        mqttclient.setServer(addr, mqtt_port);
        mqttclient.setCallback(callback);
      }
    }
    else {
      syslog("Trying to resolve MQTT host " + mqtt_host + " to IP address", 0);
      int dotLoc = mqtt_host.lastIndexOf('.');
      String tld = mqtt_host.substring(dotLoc+1);
      if(dotLoc == -1 || tld == "local"){
        if(tld == "local") mqtt_host = mqtt_host.substring(0, dotLoc);
        int mdnsretry = 0;
        while (!WiFi.hostByName(mqtt_host.c_str(), addr) && mdnsretry < 5) {
          Serial.print("...");
          mdnsretry++;
          delay(250);
        }
        while (addr.toString() == "0.0.0.0" && mdnsretry < 10) {
          Serial.print("...");
          addr = MDNS.queryHost(mqtt_host);
          mdnsretry++;
          delay(250);
        }
        if(mdnsretry < 10){
          syslog("MQTT host has IP address " + addr.toString(), 0);
          if(mqtt_tls) {
            mqttclientSecure.setServer(addr, mqtt_port);
            mqttclientSecure.setCallback(callback);
          }
          else {
            mqttclient.setServer(addr, mqtt_port);
            mqttclient.setCallback(callback);
          }
        }
        else{
          syslog("MQTT host IP resolving failed", 3);
          mqttHostError = true;
        } 
      }
      else{
        if(mqtt_tls){
          mqttclientSecure.setServer(mqtt_host.c_str(), mqtt_port);
          mqttclientSecure.setCallback(callback);
        }
        else{
          mqttclient.setServer(mqtt_host.c_str(), mqtt_port);
          mqttclient.setCallback(callback);
        }
      }
    }
  }
}

void connectMqtt() {
  if(!mqttHostError){
    // Loop until we're (re)connected
    int mqttretry = 0;
    bool disconnected = false;
    if(mqtt_tls && !clientSecureBusy){
      if(!mqttclientSecure.connected()) {
        disconnected = true;
        if(mqttWasConnected) syslog("Lost connection to secure MQTT broker", 2);
        syslog("Trying to connect to secure MQTT broker", 0);
        while(!mqttclientSecure.connected() && mqttretry < 2){
          Serial.print("...");
          String mqtt_topic = "data/devices/utility_meter";
          if (mqtt_auth) mqttclientSecure.connect(mqtt_id.c_str(), mqtt_user.c_str(), mqtt_pass.c_str(), mqtt_topic.c_str(), 1, true, "offline");
          else mqttclientSecure.connect(mqtt_id.c_str());
          mqttretry++;
          reconncount++;
          delay(250);
        }
        Serial.println("");
      }
    }
    else{
      if(!mqttclient.connected()) {
        disconnected = true;
        if(mqttWasConnected){
          reconncount++;
          syslog("Lost connection to MQTT broker", 2);
        }
        syslog("Trying to connect to MQTT broker", 0);
        while(!mqttclient.connected() && mqttretry < 2){
          Serial.print("...");
          String mqtt_topic = "data/devices/utility_meter";
          if (mqtt_auth) mqttclient.connect(mqtt_id.c_str(), mqtt_user.c_str(), mqtt_pass.c_str(), mqtt_topic.c_str(), 1, true, "offline");
          else mqttclient.connect(mqtt_id.c_str(), "data/devices/utility_meter", 1, true, "offline");
          mqttretry++;
          reconncount++;
          delay(250);
        }
        Serial.println("");
      }
    }
    if(disconnected){
      if(mqttretry < 2){
        syslog("Connected to MQTT broker", 0);
        if(mqtt_tls){
          mqttclientSecure.publish("data/devices/utility_meter", "online", true);
          mqttclientSecure.subscribe("set/devices/utility_meter/reboot");
        }
        else{
          mqttclient.publish("data/devices/utility_meter", "online", true);
          mqttclient.subscribe("set/devices/utility_meter/reboot");
        }
        mqttClientError = false;
        mqttWasConnected = true;
        reconncount = 0;
      }
      else{
        syslog("Failed to connect to MQTT broker", 3);
        mqttClientError = true;
      }
    }
  }
  else{
    //setupMqtt();
  }
}

void pubMqtt(String topic, String payload, boolean retain){
  if(mqtt_en && !mqttClientError && !clientSecureBusy){
    if(mqtt_tls){
      if(mqttclientSecure.connected()){
        mqttclientSecure.publish(topic.c_str(), payload.c_str(), retain);
      }
    }
    else{
      if(mqttclient.connected()){
        mqttclient.publish(topic.c_str(), payload.c_str(), retain);
      }
    }
  }
}

void haAutoDiscovery(boolean eraseMeter){
  if(ha_en && mqtt_en && !mqttClientError){
    if(!ha_metercreated) syslog("Performing Home Assistant MQTT autodiscovery", 0);
    int channels = sizeof(dsmrKeys)/sizeof(dsmrKeys[0]);
    for(int dsmrKey = 0; dsmrKey < channels+4; dsmrKey++){
      String chanName = "";
      DynamicJsonDocument doc(1024);
      if(dsmrKey < channels){
        chanName = String("utility_meter_") + dsmrKeys[dsmrKey][3].substring(dsmrKeys[dsmrKey][3].lastIndexOf('/')+1);
        doc["name"] = String("Utility meter ") + dsmrKeys[dsmrKey][2];
        if(dsmrKeys[dsmrKey][6] != "") doc["device_class"] = dsmrKeys[dsmrKey][6];
        if(dsmrKeys[dsmrKey][6] == "energy") doc["unit_of_measurement"] = "kWh";
        else if(dsmrKeys[dsmrKey][6] == "power") doc["unit_of_measurement"] = "kW";
        else if(dsmrKeys[dsmrKey][6] == "voltage") doc["unit_of_measurement"] = "V";
        else if(dsmrKeys[dsmrKey][6] == "current") doc["unit_of_measurement"] = "A";
        else if(dsmrKeys[dsmrKey][6] == "gas") {
          doc["unit_of_measurement"] = "m³";
          doc["state_class"] = "total_increasing";
        }
        //else doc["unit_of_measurement"] = "";
        doc["state_topic"] = dsmrKeys[dsmrKey][3];
      }
      else if(dsmrKey == channels){
        chanName = "utility_meter_total_energy_consumed";
        doc["name"] = "Utility meter Total energy consumed";
        doc["device_class"] = "energy";
        doc["unit_of_measurement"] = "kWh";
        doc["state_topic"] = "data/devices/utility_meter/total_energy_consumed";
        doc["state_class"] = "total_increasing";
        //doc["last_reset_topic"] = "1970-01-01T00:00:00+00:00";
        //doc["last_reset_topic"] = "data/devices/utility_meter/total_energy_consumed";
        //doc["last_reset_value_template"] = "1970-01-01T00:00:00+00:00";
      }
      else if(dsmrKey == channels+1){
        chanName = "utility_meter_total_energy_injected";
        doc["name"] = "Utility meter Total energy injected";
        doc["device_class"] = "energy";
        doc["unit_of_measurement"] = "kWh";
        doc["state_topic"] = "data/devices/utility_meter/total_energy_injected";
        doc["state_class"] = "total_increasing";
        //doc["last_reset_topic"] = "1970-01-01T00:00:00+00:00";
        //doc["last_reset_topic"] = "data/devices/utility_meter/total_energy_injected";
        //doc["last_reset_value_template"] = "1970-01-01T00:00:00+00:00";
      }
      else if(dsmrKey == channels+2){
        chanName = "utility_meter_total_active_power";
        doc["name"] = "Utility meter Total active power";
        doc["device_class"] = "power";
        doc["unit_of_measurement"] = "kW";
        doc["state_topic"] = "data/devices/utility_meter/total_active_power";
        doc["state_class"] = "measurement";
      }
      else {
        chanName = "";
      }
      doc["unique_id"] = chanName;
      doc["value_template"] = "{{ value_json.value }}";
      doc["availability_topic"] = "data/devices/utility_meter";
      JsonObject device  = doc.createNestedObject("device");
      JsonArray identifiers = device.createNestedArray("identifiers");
      identifiers.add("P1_utility_meter");
      device["name"] = "Utility meter";
      device["model"] = "P1 dongle for DSMR compatible utility meters";
      device["manufacturer"] = "plan-d.io";
      device["configuration_url"] = "http://" + WiFi.localIP().toString();
      device["sw_version"] = String(fw_ver/100.0);
      String configTopic = "homeassistant/sensor/" + chanName + "/config";
      String jsonOutput ="";
      //Ensure devices are erased before created again
      if(eraseMeter){
        if(chanName.length() > 0) pubMqtt(configTopic, jsonOutput, true);
        delay(100);
      }
      serializeJson(doc, jsonOutput);
      //Serial.print(configTopic);
      //Serial.print(" ");
      //Serial.println(jsonOutput);
      if(chanName.length() > 0){
        if(dsmrKey < channels){
          if(meterConfig[dsmrKey] == "1"){
            pubMqtt(configTopic, jsonOutput, true);
            //Serial.println(configTopic);
          }
        }
        else{
          pubMqtt(configTopic, jsonOutput, true);
          //Serial.println(configTopic);
        }
      }
    }
    if(debugInfo){
      //firstDebugPush = true;
      for(int i = 0; i < 10; i++){
        String chanName = "";
        DynamicJsonDocument doc(1024);
        if(i == 0){
          chanName = String(apSSID) + "_reboots";
          doc["name"] = String(apSSID ) + " Reboots";
          doc["state_topic"] = "sys/devices/" + String(apSSID) + "/reboots";
        }
        else if(i == 1){
          chanName = String(apSSID) + "_last_reset_reason";
          doc["name"] = String(apSSID ) + " Last reset reason";
          doc["state_topic"] = "sys/devices/" + String(apSSID) + "/last_reset_reason";
        }
        else if(i == 2){
          chanName = String(apSSID) + "_free_heap_size";
          doc["name"] = String(apSSID ) + " Free heap size";
          doc["unit_of_measurement"] = "kB";
          doc["state_topic"] = "sys/devices/" + String(apSSID) + "/free_heap_size";
        }
        else if(i == 3){
          chanName = String(apSSID) + "_max_allocatable_block";
          doc["name"] = String(apSSID ) + " Allocatable block size";
          doc["unit_of_measurement"] = "kB";
          doc["state_topic"] = "sys/devices/" + String(apSSID) + "/max_allocatable_block";
        }
        else if(i == 4){
          chanName = String(apSSID) + "_min_free_heap";
          doc["name"] = String(apSSID ) + " Lowest free heap size";
          doc["unit_of_measurement"] = "kB";
          doc["state_topic"] = "sys/devices/" + String(apSSID) + "/min_free_heap";
        }
        else if(i == 5){
          chanName = String(apSSID) + "_last_reset_reason_verbose";
          doc["name"] = String(apSSID ) + " Last reset reason (verbose)";
          doc["state_topic"] = "sys/devices/" + String(apSSID) + "/last_reset_reason_verbose";
        }
        else if(i == 6){
          chanName = String(apSSID) + "_syslog";
          doc["name"] = String(apSSID ) + " Syslog";
          doc["state_topic"] = "sys/devices/" + String(apSSID) + "/syslog";
        }
        else if(i == 7){
          chanName = String(apSSID) + "_ip";
          doc["name"] = String(apSSID ) + " IP";
          doc["state_topic"] = "sys/devices/" + String(apSSID) + "/ip";
        }
        else if(i == 8){
          chanName = String(apSSID) + "_firmware";
          doc["name"] = String(apSSID ) + " firmware";
          doc["state_topic"] = "sys/devices/" + String(apSSID) + "/firmware";
        }
        else if(i == 9){
          chanName = String(apSSID) + "_reboot";
          doc["name"] = String(apSSID ) + " Reboot";
          doc["payload_on"] = "{\"value\": \"true\"}";
          doc["payload_off"] = "{\"value\": \"false\"}";
          doc["command_topic"] = "set/devices/utility_meter/reboot";
        }
        doc["unique_id"] = chanName;
        doc["availability_topic"] = "data/devices/utility_meter";
        doc["value_template"] = "{{ value_json.value }}";
        JsonObject device  = doc.createNestedObject("device");
        JsonArray identifiers = device.createNestedArray("identifiers");
        identifiers.add("P1_dongle");
        device["name"] = apSSID;
        device["model"] = "P1 dongle debug monitoring";
        device["manufacturer"] = "plan-d.io";
        device["configuration_url"] = "http://" + WiFi.localIP().toString();
        device["sw_version"] = String(fw_ver/100.0);
        String configTopic = "";
        if(i == 9) configTopic = "homeassistant/switch/" + chanName + "/config";
        else configTopic = "homeassistant/sensor/" + chanName + "/config";
        String jsonOutput ="";
        //Ensure devices are erased before created again
        if(eraseMeter){
          if(chanName.length() > 0) pubMqtt(configTopic, jsonOutput, true);
          delay(100);
        }
        serializeJson(doc, jsonOutput);
        if(chanName.length() > 0) if(chanName.length() > 0) pubMqtt(configTopic, jsonOutput, true);
      }
    }
    ha_metercreated = true;
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  time_t now;
  unsigned long dtimestamp = time(&now);
  Serial.print("got mqtt message on ");
  Serial.println(String(topic));
  String messageTemp;
  for (int i = 0; i < length; i++) {
    messageTemp += (char)payload[i];
  }
  if (String(topic) == "set/devices/utility_meter/reboot") {
    StaticJsonDocument<200> doc;
    deserializeJson(doc, messageTemp);
    if(doc["value"] == "true"){
      last_reset = "Reboot requested by MQTT";
      if(saveConfig()){
        syslog("Reboot requested from MQTT", 2);
        setReboot();
      }
    }
  }
}
