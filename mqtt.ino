void setupMqtt() {
  String mqttinfo = "MQTT enabled! Will connect as " + _mqtt_id;
  if (_mqtt_auth) {
    mqttinfo = mqttinfo + " using authentication, with username " + _mqtt_user;
  }
  syslog(mqttinfo, 1);
  if(_mqtt_tls){
    mqttclientSecure.setClient(*client);
    if(_upload_throttle > 0){
      mqttclientSecure.setKeepAlive(_upload_throttle +1).setSocketTimeout(_upload_throttle +1);
    }
  }
  else {
    mqttclient.setClient(wificlient);
    if(_upload_throttle > 0){
      mqttclient.setKeepAlive(_upload_throttle +1).setSocketTimeout(_upload_throttle +1);
    }
  }
  /*Set broker location*/
  IPAddress addr;
  if (_mqtt_host.length() > 0) {
    if (addr.fromString(_mqtt_host)) {
      syslog("MQTT host has IP address " + _mqtt_host, 0);
      if(_mqtt_tls){
        mqttclientSecure.setServer(addr, _mqtt_port);
        mqttclientSecure.setCallback(callback);
      }
      else{
        mqttclient.setServer(addr, _mqtt_port);
        mqttclient.setCallback(callback);
      }
    }
    else {
      syslog("Trying to resolve MQTT host " + _mqtt_host + " to IP address", 1);
      int dotLoc = _mqtt_host.lastIndexOf('.');
      String tld = _mqtt_host.substring(dotLoc+1);
      if(dotLoc == -1 || tld == "local"){
        if(tld == "local") _mqtt_host = _mqtt_host.substring(0, dotLoc);
        int mdnsretry = 0;
        while (!WiFi.hostByName(_mqtt_host.c_str(), addr) && mdnsretry < 5) {
          Serial.print("...");
          mdnsretry++;
          delay(250);
        }
        while (addr.toString() == "0.0.0.0" && mdnsretry < 10) {
          Serial.print("...");
          addr = MDNS.queryHost(_mqtt_host);
          mdnsretry++;
          delay(250);
        }
        if(mdnsretry < 10){
          syslog("MQTT host has IP address " + addr.toString(), 1);
          if(_mqtt_tls) {
            mqttclientSecure.setServer(addr, _mqtt_port);
            mqttclientSecure.setCallback(callback);
          }
          else {
            mqttclient.setServer(addr, _mqtt_port);
            mqttclient.setCallback(callback);
          }
        }
        else{
          syslog("MQTT host IP resolving failed", 3);
          mqttHostError = true;
        } 
      }
      else{
        if(_mqtt_tls){
          mqttclientSecure.setServer(_mqtt_host.c_str(), _mqtt_port);
          mqttclientSecure.setCallback(callback);
        }
        else{
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
    if(_mqtt_tls && !clientSecureBusy){
      if(!mqttclientSecure.connected()) {
        disconnected = true;
        if(mqttWasConnected){
          if(!mqttPaused) syslog("Lost connection to secure MQTT broker", 2);
        }
        syslog("Trying to connect to secure MQTT broker", 0);
        while(!mqttclientSecure.connected() && mqttretry < 2){
          Serial.print("...");
          String mqtt_topic = "data/devices/utility_meter";
          if (_mqtt_auth) mqttclientSecure.connect(_mqtt_id.c_str(), _mqtt_user.c_str(), _mqtt_pass.c_str(), mqtt_topic.c_str(), 1, true, "offline");
          else mqttclientSecure.connect(_mqtt_id.c_str());
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
          //reconncount++;
          if(!mqttPaused) syslog("Lost connection to MQTT broker", 2);
        }
        syslog("Trying to connect to MQTT broker", 0);
        while(!mqttclient.connected() && mqttretry < 2){
          Serial.print("...");
          String mqtt_topic = "data/devices/utility_meter";
          if (_mqtt_auth) mqttclient.connect(_mqtt_id.c_str(), _mqtt_user.c_str(), _mqtt_pass.c_str(), mqtt_topic.c_str(), 1, true, "offline");
          else mqttclient.connect(_mqtt_id.c_str(), "data/devices/utility_meter", 1, true, "offline");
          mqttretry++;
          reconncount++;
          delay(250);
        }
        Serial.println("");
      }
    }
    if(disconnected){
      if(mqttretry < 2){
        syslog("Connected to MQTT broker", 1);
        if(mqttPaused) mqttPaused = false;
        if(_mqtt_tls){
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
  if(_mqtt_en && !mqttClientError && !clientSecureBusy){
    if(_mqtt_tls){
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
