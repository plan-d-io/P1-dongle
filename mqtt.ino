void setupMqtt() {
  String mqttinfo = "MQTT enabled! Will connect as " + _mqtt_id;
  if (_mqtt_auth) {
    mqttinfo = mqttinfo + " using authentication, with username " + _mqtt_user;
  }
  syslog(mqttinfo, 1);
  if(_mqtt_tls){
    if(mqttWasConnected) mqttclientSecure.disconnect();
    mqttWasConnected = false;
    mqttclientSecure.setClient(*client);
    if(_upload_throttle > 0){
      if(_realto_en) mqttclientSecure.setKeepAlive(_realtoThrottle*2).setSocketTimeout(_realtoThrottle*2);
      else mqttclientSecure.setKeepAlive(_upload_throttle*2).setSocketTimeout(_upload_throttle*2); 
      mqttclientSecure.setBufferSize(2048);
    }
  }
  else {
    if(mqttWasConnected) mqttclient.disconnect();
    mqttWasConnected = false;
    mqttclient.setClient(wificlient);
    if(_upload_throttle > 0){
      if(_realto_en) mqttclient.setKeepAlive(_realtoThrottle*2).setSocketTimeout(_realtoThrottle*2);
      else mqttclient.setKeepAlive(_upload_throttle*2).setSocketTimeout(_upload_throttle*2);
      mqttclient.setBufferSize(2048);
    }
  }
  /*Set broker location*/
  IPAddress addr;
  if (_mqtt_host.length() > 0) { //check if hostname is filled in
    /*If a valid IP is filled in, use this to set the client*/
    if (addr.fromString(_mqtt_host)) {
      syslog("MQTT host has IP address " + _mqtt_host, 0);
      if(_mqtt_tls){
        mqttHostError = false;
        mqttclientSecure.setServer(addr, _mqtt_port);
        mqttclientSecure.setCallback(callback);
      }
      else{
        mqttHostError = false;
        mqttclient.setServer(addr, _mqtt_port);
        mqttclient.setCallback(callback);
      }
    }
    /*If it's a hostname, resolve it to its IP*/
    else {
      syslog("Resolving MQTT host " + _mqtt_host + " to IP address", 1);
      int dotLoc = _mqtt_host.lastIndexOf('.');
      String tld = _mqtt_host.substring(dotLoc+1);
      if(dotLoc == -1 || tld == "local"){
        if(tld == "local") _mqtt_host = _mqtt_host.substring(0, dotLoc);
        int mdnsretry = 0;
        while (!WiFi.hostByName(_mqtt_host.c_str(), addr) && mdnsretry < 5) {
          syslog("Resolving... ", 1);
          mdnsretry++;
          delay(250);
        }
        while (addr.toString() == "0.0.0.0" && mdnsretry < 10) {
          syslog("Resolving... ", 1);
          addr = MDNS.queryHost(_mqtt_host);
          mdnsretry++;
          delay(250);
        }
        if(mdnsretry < 10){
          syslog("MQTT host has IP address " + addr.toString(), 1);
          if(_mqtt_tls) {
            mqttHostError = false;
            mqttclientSecure.setServer(addr, _mqtt_port);
            mqttclientSecure.setCallback(callback);
          }
          else {
            mqttHostError = false;
            mqttclient.setServer(addr, _mqtt_port);
            mqttclient.setCallback(callback);
          }
        }
        else{
          mqttHostError = true;
          syslog("MQTT host IP resolving failed", 3);
          if(unitState < 6) unitState = 5;
        } 
      }
      else{
        if(_mqtt_tls){
          mqttHostError = false;
          mqttclientSecure.setServer(_mqtt_host.c_str(), _mqtt_port);
          mqttclientSecure.setCallback(callback);
        }
        else{
          mqttHostError = false;
          mqttclient.setServer(_mqtt_host.c_str(), _mqtt_port);
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
    if(_mqtt_tls){
      if(mqttClientError) mqttclientSecure.disconnect();
      if(!mqttclientSecure.connected()) {
        disconnected = true;
        if(mqttWasConnected){
          if(!mqttPaused && !mqttWasPaused){
            syslog("Lost connection to secure MQTT broker", 4);
            if(unitState < 6) unitState = 5;
          }
        }
        syslog("Trying to connect to secure MQTT broker", 0);
        while(!mqttclientSecure.connected() && mqttretry < 2){
          Serial.print("...");
          String mqtt_topic = "plan-d/" + String(apSSID);
          if (_mqtt_auth) mqttclientSecure.connect(_mqtt_id.c_str(), _mqtt_user.c_str(), _mqtt_pass.c_str(), mqtt_topic.c_str(), 1, true, "offline");
          else mqttclientSecure.connect(_mqtt_id.c_str());
          mqttretry++;
          reconncount++;
          delay(250);
        }
        secureClientError++;
        Serial.println("");
      }
    }
    else{
      if(mqttClientError) mqttclient.disconnect();
      if(!mqttclient.connected()) {
        disconnected = true;
        if(mqttWasConnected){
          if(!mqttPaused && !mqttWasPaused){
            syslog("Lost connection to MQTT broker", 4);
            if(unitState < 6) unitState = 5;
          }
        }
        syslog("Trying to connect to MQTT broker", 0);
        while(!mqttclient.connected() && mqttretry < 2){
          Serial.print("...");
          String mqtt_topic = _mqtt_prefix.substring(0, _mqtt_prefix.length()-1);
          if (_mqtt_auth) mqttclient.connect(_mqtt_id.c_str(), _mqtt_user.c_str(), _mqtt_pass.c_str(), mqtt_topic.c_str(), 1, true, "offline");
          else mqttclient.connect(_mqtt_id.c_str(), mqtt_topic.c_str(), 1, true, "offline");
          mqttretry++;
          reconncount++;
          delay(250);
        }
        Serial.println("");
      }
    }
    if(disconnected){
      if(mqttretry < 2){
        if(!mqttPaused) syslog("Connected to MQTT broker", 1);
        if(unitState < 6) unitState = 4;
        if(mqttPaused) mqttPaused = false;
        String availabilityTopic = _mqtt_prefix.substring(0, _mqtt_prefix.length()-1);
        if(_mqtt_tls){
          mqttclientSecure.publish(availabilityTopic.c_str(), "online", true);
          mqttclientSecure.publish((availabilityTopic +"/sys/config").c_str(), returnBasicConfig().c_str(), true);
          mqttclientSecure.subscribe((availabilityTopic+"/set/reboot").c_str());
          mqttclientSecure.subscribe((availabilityTopic+"/set/config").c_str());
          secureClientError = 0;
          if(_rebootSecure > 0){
            _rebootSecure = 0;
            saveConfig();
          }
        }
        else{
          mqttclient.publish(availabilityTopic.c_str(), "online", true);
          mqttclient.publish((availabilityTopic +"/sys/config").c_str(), returnBasicConfig().c_str(), true);
          mqttclient.subscribe((availabilityTopic+"/set/reboot").c_str());
          mqttclient.subscribe((availabilityTopic+"/set/config").c_str());
        }
        mqttClientError = false;
        pushSyslog(30);
        if(debugInfo && !mqttWasConnected){
          hadebugDevice(true);
          delay(500);
          hadebugDevice(false);
          delay(500);
          getHeapDebug();
        }
        mqttWasConnected = true;
        reconncount = 0;
      }
      else{
        syslog("Failed to connect to MQTT broker", 4);
        mqttClientError = true;
        if(unitState < 6) unitState = 5;
      }
    }
  }
}

bool pubMqtt(String topic, String payload, boolean retain){
  bool pushed = false;
  if(mqttDebug){
    Serial.print("MQTT push to ");
    Serial.println(topic);
  }
  if(_mqtt_en && !mqttClientError && !mqttHostError && !mqttPaused && !clientSecureBusy){
    if(_mqtt_tls){
      if(mqttclientSecure.connected()){
        if(mqttclientSecure.publish(topic.c_str(), payload.c_str(), retain)){
          mqttPushFails = 0;
          pushed = true;
        }
        else mqttPushFails++;
      }
      else{
        mqttClientError = true;
        sinceConnCheck = 60000;
      }
    }
    else{
      if(mqttclient.connected()){
        if(mqttclient.publish(topic.c_str(), payload.c_str(), retain)){
          mqttPushFails = 0;
          pushed = true;
        }
        else mqttPushFails++;
      }
      else{
        mqttClientError = true;
        sinceConnCheck = 60000;
      }
    }
  }
  return pushed;
}

void callback(char* topic, byte* payload, unsigned int length) {
  time_t now;
  unsigned long dtimestamp = time(&now);
  //String availabilityTopic = _mqtt_prefix.substring(0, _mqtt_prefix.length()-1);
  String dtopic = "set/devices/" + _ha_device;
  if(mqttDebug){
    Serial.print("got mqtt message on ");
    Serial.print(String(topic));
  }
  String messageTemp;
  for (int i = 0; i < length; i++) {
    messageTemp += (char)payload[i];
  }
  if(mqttDebug){
    Serial.print(", ");
    Serial.println(messageTemp);
  }
  if (String(topic) == dtopic + "/reboot") {
    StaticJsonDocument<200> doc;
    deserializeJson(doc, messageTemp);
    if(doc["value"] == "true"){
      saveResetReason("Reboot requested by MQTT");
      if(saveConfig()){
        syslog("Reboot requested from MQTT", 2);
        pubMqtt(dtopic + "/set/reboot", "{\"value\": \"false\"}", false);
        pubMqtt("sys/devices/" + String(apSSID) + "/reboot", "{\"value\": \"true\"}", false);
        delay(500);
        setReboot();
      }
    }
  }
  if (String(topic) == dtopic + "/config") {
    syslog("Got config update over MQTT", 1);
    String configResponse;
    processConfigJson(messageTemp, configResponse, true);
  }
}
