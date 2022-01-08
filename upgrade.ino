boolean checkUpdate(){
  clientSecureBusy = true;
  boolean needUpdate = false;
  boolean mqttPaused;
  if(mqttclientSecure.connected()){
    Serial.println("Disconnecting TLS MQTT connection");
    mqttclientSecure.disconnect();
    mqttPaused = true;
  }
  if(bundleLoaded){
    Serial.print("Checking repository for firmware update... ");
    if (https.begin(*client, "https://raw.githubusercontent.com/plan-d-io/P1-dongle/main/version")) {  
      int httpCode = https.GET();
      Serial.println(httpCode);
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = https.getString();
          onlineVersion = atoi(payload.c_str());
        }
      } else {
        Serial.printf("GET... failed, error: %s", https.errorToString(httpCode).c_str());
        Serial.println("");
      }
      https.end();
    } else {
      Serial.println("unable to connect");
    }
  }
  client->stop();
  clientSecureBusy = false;
  if(mqttPaused){
    sinceConnCheck = 10000;
  }
  //Serial.println("");
  Serial.print("Current firmware: ");
  Serial.print(fw_ver/100.0);
  Serial.print(", online version: ");
  Serial.println(onlineVersion/100.0);
  if(onlineVersion > fw_ver){
    needUpdate = true;
    Serial.println("Firmware update available");
  }
  else Serial.println("No firmware update available");
  return needUpdate;
}

boolean startUpdate(){
  if(fw_ver < onlineVersion || update_start){
    clientSecureBusy = true;
    boolean needUpdate, mqttPaused;
    if(mqttclientSecure.connected()){
      Serial.println("Disconnecting TLS MQTT connection");
      mqttclientSecure.disconnect();
      mqttPaused = true;
    }
    if(bundleLoaded){
      //String baseUrl = "https://raw.githubusercontent.com/plan-d-io/P1-dongle/main";
      //String fileUrl = baseUrl + "/bin/P1-dongle-V" + String(onlineVersion/100.0) +"ino.bin";
      String baseUrl ="https://raw.githubusercontent.com/plan-d-io/P1-dongle/main/bin/P1-dongle-V";
      String fileUrl = baseUrl + String(fw_new/100.0) +".ino.bin";
      Serial.println("Getting new firmware over HTTPS/TLS");
      Serial.println(fileUrl);
      if (https.begin(*client, fileUrl)) {  
        int httpCode = https.GET();
        Serial.println(httpCode);
        if (httpCode > 0) {
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            long contentLength = https.getSize();
            Serial.print("Firmware size: ");
            Serial.println(contentLength);
            // Check if there is enough to OTA Update
            bool canBegin = Update.begin(contentLength);
            // If yes, begin
            if (canBegin) {
              unitState = 0;
              Serial.println("Begin OTA. This may take 2 - 5 mins to complete. Things might be quiet for a while.. Patience!");
              // No activity would appear on the Serial monitor
              // So be patient. This may take 2 - 5mins to complete
              size_t written = Update.writeStream(*client);
              if (written == contentLength) {
                Serial.println("Written : " + String(written) + " successfully");
              } else {
                Serial.println("Written only : " + String(written) + "/" + String(contentLength) + ". Retry?" );
                // retry??
                // execOTA();
              }
              if (Update.end()) {
                Serial.println("OTA done!");
                if (Update.isFinished()) {
                  Serial.println("Update successfully completed. Rebooting.");
                  fw_ver = onlineVersion;
                  update_start = false;
                  update_finish = true;
                  saveConfig();
                  delay(500);
                  ESP.restart();
                } else {
                  Serial.println("Update not finished? Something went wrong!");
                }
              } else {
                Serial.println("Error Occurred. Error #: " + String(Update.getError()));
              }
            } 
            else {
              // Not enough space to begin OTA
              Serial.println("Not enough space to begin OTA");
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
            Serial.printf("GET... failed, error: %s", https.errorToString(httpCode).c_str());
          }
        } 
        else {
          Serial.printf("GET... failed, error: %s", https.errorToString(httpCode).c_str());
        }
        https.end(); 
      } 
      else {
        Serial.print("Unable to connect");
      }
    }
    client->stop();
    clientSecureBusy = false;
    if(mqttPaused){
      sinceConnCheck = 10000;
    }
    return true;
  }
  else{
    Serial.println("No upgrade available");
    return false;
  }
  return true;
}

boolean finishUpdate(){
  clientSecureBusy = true;
  boolean mqttPaused;
  boolean filesUpdated = false;
  if(mqttclientSecure.connected()){
    Serial.println("Disconnecting TLS MQTT connection");
    mqttclientSecure.disconnect();
    mqttPaused = true;
  }
  if(bundleLoaded){
    String baseUrl = "https://raw.githubusercontent.com/plan-d-io/P1-dongle/main";
    String fileUrl = baseUrl + "/bin/files";
    String payload;
    if (https.begin(*client, fileUrl)) {  
      int httpCode = https.GET();
      if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          payload = https.getString();
        }
      } 
      else {
        Serial.printf("GET... failed, error: %s", https.errorToString(httpCode).c_str());
      }
      https.end();
      unsigned long eof = payload.lastIndexOf('\n');
      if(eof > 0){
        Serial.println("Downloading files from repo:");
        unsigned long delimStart = 0;
        unsigned long delimEnd = 0;
        while(delimEnd < eof){
          delimEnd = payload.indexOf('\n', delimStart);
          String s = "/" + payload.substring(delimStart, delimEnd);
          delimStart = delimEnd+1;
          fileUrl = baseUrl + "/data" + s;
          Serial.println(fileUrl);
          File f = SPIFFS.open(s, FILE_WRITE);
          Serial.println(s);
          if (f) {
            if (https.begin(*client, fileUrl)) {
              int httpCode = https.GET();
              if (httpCode > 0) {
                if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
                  long contentLength = https.getSize();
                  Serial.print("File size: ");
                  Serial.println(contentLength);
                  Serial.println("Begin download");
                  size_t written = https.writeToStream(&f);
                  if (written == contentLength) {
                    Serial.println("Written : " + String(written) + " successfully");
                    filesUpdated = true;
                  }
                }
              } 
              else {
                Serial.printf("GET... failed, error: %s", https.errorToString(httpCode).c_str());
              }
              https.end();
            }
          }
          else{
            Serial.println("Could not write file!");
          }
        }
      }   
    } 
    else {
      Serial.print("Unable to connect");
    }
  }
  client->stop();
  clientSecureBusy = false;
  //update_finish = false;
  if(filesUpdated){
    update_finish = false;
    saveConfig();
    delay(500);
    ESP.restart();
  }
  if(mqttPaused){
    sinceConnCheck = 10000;
  }
  return true;
}
