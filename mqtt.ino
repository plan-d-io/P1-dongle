void setupMqtt() {
  Serial.println("MQTT enabled!");
  Serial.print("Will connect as ");
  Serial.print(mqtt_id);
  if (mqtt_auth) {
    Serial.print(" using authentication, with username ");
    Serial.println(mqtt_user);
  }
  else {
    Serial.println("");
  }
  if(mqtt_tls) mqttclientSecure.setClient(*client);
  else mqttclient.setClient(wificlient);
  /*Set broker location*/
  IPAddress addr;
  if (mqtt_host.length() > 0) {
    if (addr.fromString(mqtt_host)) {
      Serial.print("MQTT host is IP address ");
      Serial.println(addr);
      if(mqtt_tls) mqttclientSecure.setServer(addr, mqtt_port);
      else mqttclient.setServer(addr, mqtt_port);
    }
    else {
      Serial.print("Trying to resolve MQTT host ");
      Serial.print(mqtt_host);
      Serial.print(" to IP address ");
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
          Serial.println(addr);
          if(mqtt_tls) mqttclientSecure.setServer(addr, mqtt_port);
          else mqttclient.setServer(addr, mqtt_port); 
        }
        else{
          Serial.println(" failed.");
          mqttHostError = true;
        } 
      }
      else{
        Serial.println("... skipped");
        if(mqtt_tls) mqttclientSecure.setServer(mqtt_host.c_str(), mqtt_port);
        else mqttclient.setServer(mqtt_host.c_str(), mqtt_port);
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
        Serial.print("Trying to connect to secure MQTT broker");
        while(!mqttclientSecure.connected() && mqttretry < 5){
          Serial.print("...");
          if (mqtt_auth) mqttclientSecure.connect(mqtt_id.c_str(), mqtt_user.c_str(), mqtt_pass.c_str());
          else mqttclientSecure.connect(mqtt_id.c_str());
          mqttretry++;
          delay(250);
        }
      }
    }
    else{
      if(!mqttclient.connected()) {
        disconnected = true;
        Serial.print("Trying to connect to MQTT broker");
        while(!mqttclient.connected() && mqttretry < 5){
          Serial.print("...");
          if (mqtt_auth) mqttclient.connect(mqtt_id.c_str(), mqtt_user.c_str(), mqtt_pass.c_str());
          else mqttclient.connect(mqtt_id.c_str());
          mqttretry++;
          delay(250);
        }
      }
    }
    if(disconnected){
      if(mqttretry < 5){
        Serial.println(" success!");
      }
      else{
        Serial.println(" failed.");
        mqttClientError = true;
      }
    }
  }
  else{
    //setupMqtt();
  }
}

void pubMqtt(String topic, String payload, boolean retain){
  if(mqtt_en){
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
  if(ha_en && mqtt_en){
    int channels = sizeof(dsmrKeys)/sizeof(dsmrKeys[0]);
    for(int dsmrKey = 0; dsmrKey < channels+4; dsmrKey++){
      String chanName = "";
      DynamicJsonDocument doc(1024);
      if(dsmrKey < channels){
        chanName = dsmrKeys[dsmrKey][3].substring(dsmrKeys[dsmrKey][3].lastIndexOf('/')+1);
        doc["name"] = dsmrKeys[dsmrKey][2];
        if(dsmrKeys[dsmrKey][6] != "") doc["device_class"] = dsmrKeys[dsmrKey][6];
        if(dsmrKeys[dsmrKey][6] == "energy") doc["unit_of_measurement"] = "kWh";
        else if(dsmrKeys[dsmrKey][6] == "power") doc["unit_of_measurement"] = "kW";
        else if(dsmrKeys[dsmrKey][6] == "voltage") doc["unit_of_measurement"] = "V";
        else if(dsmrKeys[dsmrKey][4] == "naturalGasImport") doc["unit_of_measurement"] = "m³";
        //else doc["unit_of_measurement"] = "";
        doc["state_topic"] = dsmrKeys[dsmrKey][3];
      }
      else if(dsmrKey == channels){
        chanName = "total_energy_consumed";
        doc["name"] = "Total energy consumed";
        doc["device_class"] = "energy";
        doc["unit_of_measurement"] = "kWh";
        doc["state_topic"] = "data/devices/utility_meter/" + chanName;
        doc["state_class"] = "measurement";
        //doc["last_reset_topic"] = "1970-01-01T00:00:00+00:00";
        doc["last_reset_topic"] = "data/devices/utility_meter/" + chanName;
        doc["last_reset_value_template"] = "1970-01-01T00:00:00+00:00";
      }
      else if(dsmrKey == channels+1){
        chanName = "total_energy_injected";
        doc["name"] = "Total energy injected";
        doc["device_class"] = "energy";
        doc["unit_of_measurement"] = "kWh";
        doc["state_topic"] = "data/devices/utility_meter/" + chanName;
        doc["state_class"] = "measurement";
        //doc["last_reset_topic"] = "1970-01-01T00:00:00+00:00";
        doc["last_reset_topic"] = "data/devices/utility_meter/" + chanName;
        doc["last_reset_value_template"] = "1970-01-01T00:00:00+00:00";
      }
      else if(dsmrKey == channels+2){
        chanName = "total_active_power";
        doc["name"] = "Total active power";
        doc["device_class"] = "power";
        doc["unit_of_measurement"] = "kW";
        doc["state_topic"] = "data/devices/utility_meter/" + chanName;
        doc["state_class"] = "measurement";
      }
      else {
        chanName = "";
      }
      doc["unique_id"] = chanName;
      doc["value_template"] = "{{ value_json.value }}";
      JsonObject device  = doc.createNestedObject("device");
      JsonArray identifiers = device.createNestedArray("identifiers");
      identifiers.add("P1_utility_meter");
      device["name"] = "P1 utility meter";
      device["model"] = "P1 dongle for DSMR compatible utility meters";
      device["manufacturer"] = "plan-d.io";
      String configTopic = "homeassistant/sensor/" + chanName + "/config";
      String jsonOutput ="";
      if(!eraseMeter) serializeJson(doc, jsonOutput);
      if(chanName.length() > 0) pubMqtt(configTopic, jsonOutput, true);
    }
    if(debugInfo){
      for(int i = 0; i < 5; i++){
        String chanName = "";
        DynamicJsonDocument doc(1024);
        if(i == 0){
          chanName = String(apSSID ) + "_reboots";
          doc["name"] = "Reboots";
          doc["state_topic"] = "sys/devices/" + String(apSSID) + "/reboots";
        }
        else if(i == 1){
          chanName = String(apSSID ) + "_last_reset_reason";
          doc["name"] = "Last reset reason";
          doc["state_topic"] = "sys/devices/" + String(apSSID) + "/last_reset_reason";
        }
        else if(i == 2){
          chanName = String(apSSID ) + "_free_heap_size";
          doc["name"] = "Free heap size";
          doc["unit_of_measurement"] = "kB";
          doc["state_topic"] = "sys/devices/" + String(apSSID) + "/free_heap_size";
        }
        else if(i == 3){
          chanName = String(apSSID ) + "_max_allocatable_block";
          doc["name"] = "Allocatable block size";
          doc["unit_of_measurement"] = "kB";
          doc["state_topic"] = "sys/devices/" + String(apSSID) + "/max_allocatable_block";
        }
        else if(i == 4){
          chanName = String(apSSID ) + "_min_free_heap";
          doc["name"] = "Lowest free heap size";
          doc["unit_of_measurement"] = "kB";
          doc["state_topic"] = "sys/devices/" + String(apSSID) + "/min_free_heap";
        }
        doc["unique_id"] = chanName;
        doc["value_template"] = "{{ value_json.value }}";
        JsonObject device  = doc.createNestedObject("device");
        JsonArray identifiers = device.createNestedArray("identifiers");
        identifiers.add("P1_dongle");
        device["name"] = apSSID;
        device["model"] = "P1 dongle debug monitoring";
        device["manufacturer"] = "plan-d.io";
        String configTopic = "homeassistant/sensor/" + chanName + "/config";
        String jsonOutput ="";
        if(!eraseMeter) serializeJson(doc, jsonOutput);
        pubMqtt(configTopic, jsonOutput, true);
      }
    }
    ha_metercreated = true;
  }
}
