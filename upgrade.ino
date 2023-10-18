boolean checkUpdate(){
  if(_update_autoCheck){
    clientSecureBusy = true;
    bool needUpdate = false;
    if(_mqtt_tls){
      if(mqttclientSecure.connected()){
        syslog("Disconnecting TLS MQTT connection to perform firmware version check", 0);
        String mqtt_topic = "plan-d/" + String(apSSID);
        mqttclientSecure.publish(mqtt_topic.c_str(), "offline", true);
        mqttclientSecure.disconnect();
        mqttPaused = true;
      }
    }
    if(bundleLoaded){
      syslog("Checking repository for firmware update... ", 0);
      String checkUrl = "https://raw.githubusercontent.com/plan-d-io/P1-dongle/";
      if(_dev_fleet) checkUrl += "develop/version";
      else if(_alpha_fleet) checkUrl += "alpha/version";
      else if(_v2_fleet) checkUrl += "V2-0/version";
      else checkUrl += "main/version";
      syslog("Connecting to " + checkUrl, 0);
      if (https.begin(*client, checkUrl)) {  
        int httpCode = https.GET();
        if (httpCode > 0) {
          // HTTP header has been send and Server response header has been handled
          // file found at server
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            String payload = https.getString();
            onlineVersion = atoi(payload.c_str());
          }
        } else {
          syslog("Could not connect to update repository, HTTPS code " + String(https.errorToString(httpCode)), 2);
        }
        https.end();
      } else {
        syslog("Unable to connect to update repository", 2);
      }
    }
    client->stop();
    clientSecureBusy = false;
    if(mqttPaused){
      sinceConnCheck = 10000;
    }
    //Serial.println("");
    syslog("Current firmware: " + String(fw_ver/100.0) + ", online version: " + String(onlineVersion/100.0), 0);
    if(onlineVersion > fw_ver){
      needUpdate = true;
      syslog("Firmware update available", 1);
    }
    else{
      syslog("No firmware update available", 1);
      if(_mqtt_en) connectMqtt();
    }
    mqttPaused = false;
    return needUpdate;
  }
}

boolean startUpdate(){
  if(_update_auto){
    if(fw_ver < onlineVersion || _update_start){
      syslog("Preparing firmware upgrade", 1);
      clientSecureBusy = true;
      mqttPaused = true;
      if(mqttclientSecure.connected()){
        syslog("Disconnecting TLS MQTT connection to fetch firmware upgrade", 2);
        mqttclientSecure.disconnect();
        mqttPaused = true;
      }
      if(bundleLoaded){
        String baseUrl ="https://raw.githubusercontent.com/plan-d-io/P1-dongle/";
        if(_dev_fleet) baseUrl += "develop/bin/connect-p1dongle-firmware";
        else if(_alpha_fleet) baseUrl += "alpha/connect-p1dongle-firmware";
        else if(_v2_fleet) baseUrl += "V2-0/bin/connect-p1dongle-firmware";
        else baseUrl += "main/bin/connect-p1dongle-firmware";
        String fileUrl = baseUrl + ".ino.bin"; //leaving this split up for now if we later want to do versioning in the filename
        syslog("Getting new firmware over HTTPS/TLS", 0);
        syslog("Found new firmware at "+ fileUrl, 0);
        if (https.begin(*client, fileUrl)) {  
          int httpCode = https.GET();
          if (httpCode > 0) {
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
              long contentLength = https.getSize();
              syslog("Firmware size: " + String(contentLength), 0);
              // Check if there is enough to OTA Update
              bool canBegin = Update.begin(contentLength);
              // If yes, begin
              if (canBegin) {
                unitState = -1;
                blinkLed();
                syslog("Beginning firmware upgrade. This may take 2 - 5 mins to complete. Things might be quiet for a while.. Patience!", 2);
                // No activity would appear on the Serial monitor
                // So be patient. This may take 2 - 5mins to complete
                size_t written = Update.writeStream(*client);
                if (written == contentLength) {
                  syslog("Written : " + String(written) + " successfully", 0);
                } else {
                  syslog("Written only : " + String(written) + "/" + String(contentLength) + ". Retry?", 3);
                  _update_start = false;
                }
                if (Update.end()) {
                  if (Update.isFinished()) {
                    syslog("Firmware upgrade successfully completed. Rebooting to finish update.", 1);
                    saveResetReason("Firmware upgrade successfully completed. Rebooting to finish update");
                    fw_ver = onlineVersion;
                    _update_start = false;
                    _update_finish = true;
                    saveConfig();
                    preferences.end();
                    SPIFFS.end();
                    delay(500);
                    //ESP.restart();
                    forcedReset();
                  } else {
                    syslog("Firmware upgrade not finished? Something went wrong!", 3);
                    _update_start = false;
                  }
                } else {
                  syslog("Firmware upgrade error occurred. Error #: " + String(Update.getError()), 3);
                  _update_start = false;
                }
              } 
              else {
                // Not enough space to begin OTA
                syslog("Not enough space to begin OTA upgrade", 3);
                _update_start = false;
                client->flush();
              }
              while (client->available()) {
                // read line till /n
                String line = client->readString();
                // remove space, to check if the line is end of headers
                Serial.print(line);
              }
            }
            else{
              syslog("Could not connect to repository, HTTPS code " + String(https.errorToString(httpCode)), 2);
              _update_start = false;
              if(unitState < 6) unitState = 5;
            }
          } 
          else {
            syslog("Could not connect to repository, HTTPS code " + String(https.errorToString(httpCode)), 2);
            _update_start = false;
            if(unitState < 6) unitState = 5;
          }
          https.end(); 
        } 
        else {
          Serial.print("Unable to connect");
          _update_start = false;
          if(unitState < 6) unitState = 5;
        }
      }
      client->stop();
      clientSecureBusy = false;
      if(mqttPaused){
        sinceConnCheck = 10000;
      }
      _update_start = false;
      return true;
    }
    else{
      syslog("No firmware upgrade available", 0);
      _update_start = false;
      if(unitState < 5) unitState = 4;
      return false;
    }
    _update_start = false;
    mqttPaused = false;
    saveConfig();
    delay(500);
    return true;
  }
}

boolean finishUpdate(bool restore){
  if(!_v2_fleet){
    clientSecureBusy = true;
    boolean filesUpdated = false;
    if(mqttclientSecure.connected()){
      syslog("Disconnecting TLS MQTT connection to fetch update", 2);
      mqttclientSecure.disconnect();
      mqttPaused = true;
    }
    if(bundleLoaded){
      syslog("Finishing upgrade. Preparing to download static files.", 1);
      String baseUrl = "https://raw.githubusercontent.com/plan-d-io/P1-dongle/";
      if(_dev_fleet) baseUrl += "develop";
      else if(_alpha_fleet) baseUrl += "alpha";
      else baseUrl += "main";
      String fileUrl = baseUrl + "/bin/";
      if(restore) fileUrl += "restore";
      else fileUrl += "files";
      String payload;
      if (https.begin(*client, fileUrl)) {
        int httpCode = https.GET();
        if (httpCode > 0) {
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            payload = https.getString();
          }
        } 
        else {
          syslog("Could not connect to repository, HTTPS code " + String(https.errorToString(httpCode)), 2);
        }
        https.end();
        unsigned long eof = payload.lastIndexOf('\n');
        if(eof > 0){
          syslog("Downloading static files", 2);
          unitState = -1;
          blinkLed();
          unsigned long delimStart = 0;
          unsigned long delimEnd = 0;
          while(delimEnd < eof){
            delimEnd = payload.indexOf('\n', delimStart);
            String s = "/";
            String temp = payload.substring(delimStart, delimEnd);
            if(restore) s += payload.substring(delimStart, delimEnd-1);
            else s += payload.substring(delimStart, delimEnd-1);
            delimStart = delimEnd+1;
            fileUrl = baseUrl + "/data" + s;
            Serial.println(fileUrl);
            if (s) {
              if (https.begin(*client, fileUrl)) {
                int httpCode = https.GET();
                if (httpCode > 0) {
                  if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
                    SPIFFS.remove(s);
                    File f = SPIFFS.open(s, FILE_WRITE);
                    long contentLength = https.getSize();
                    Serial.print("File size: ");
                    Serial.println(contentLength);
                    Serial.println("Begin download");
                    size_t written = https.writeToStream(&f);
                    if (written == contentLength) {
                      Serial.println("Written : " + String(written) + " successfully");
                      filesUpdated = true;
                    }
                    f.close();
                  }
                  else{
                    syslog("Could not fetch file, HTTPS code " + String(httpCode), 2);
                    if(httpCode == 400 || httpCode == 404){ //temp fix till we can figure out the issue with non-deterministic behaviour of line-endings (github encoding?)
                      Serial.println("retrying");
                      https.end();
                      s = "/";
                      s += temp;
                      delimStart = delimEnd+1;
                      fileUrl = baseUrl + "/data" + s;
                      Serial.println(fileUrl);
                      if (s) {
                        if (https.begin(*client, fileUrl)) {
                          httpCode = https.GET();
                          Serial.println(httpCode);
                          if (httpCode > 0) {
                            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
                              SPIFFS.remove(s);
                              File f = SPIFFS.open(s, FILE_WRITE);
                              long contentLength = https.getSize();
                              Serial.print("File size: ");
                              Serial.println(contentLength);
                              Serial.println("Begin download");
                              size_t written = https.writeToStream(&f);
                              if (written == contentLength) {
                                Serial.println("Written : " + String(written) + " successfully");
                                filesUpdated = true;
                              }
                              f.close();
                            }
                          }
                        }
                        else{
                          Serial.println("could not start client");
                        }
                      }
                      else {
                        Serial.print("No s: ");
                        Serial.println(s);
                      }                    
                    }
                  }
                } 
                else {
                  syslog("Could not connect to repository, HTTPS code " + String(https.errorToString(httpCode)), 2);
                }
                https.end();
              }
              else {
                syslog("Could not establish connection", 2);
              }
            }
            else{
              syslog("Could not write files!", 3);
            }
          }
        }   
      } 
      else {
        syslog("Unable to connect to repository", 2);
      }
    }
    client->stop();
    clientSecureBusy = false;
    _update_finish = false;
    if(filesUpdated){
      _update_finish = false;
      if(_restore_finish) _restore_finish = false;
      syslog("Static files successfully updated. Rebooting to finish update.", 1);
      saveResetReason("Static files successfully updated. Rebooting to finish update.");
      saveConfig();
      SPIFFS.end();
      delay(500);
      //ESP.restart();
      forcedReset();
    }
    if(mqttPaused){
      sinceConnCheck = 10000;
    }
  }
  else{
    clientSecureBusy = false;
    _update_finish = false;
  }
  mqttPaused = false;
  saveConfig();
  //if(unitState < 6) unitState = 5; //what?
  delay(500);
  return true;
}
