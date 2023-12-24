void syslog(String msg, int level){
  String logmsg;
  unsigned long dtimestamp; 
  if(timeSet){
    dtimestamp = printUnixTime();
    logmsg = printLocalTime(false) + " ";
  }
  if(level == 0 || level == 1) logmsg = logmsg + "INFO: "; //level 0 is used for INFO messages which do not need to be saved to SPIFFS
  else if(level == 2) logmsg = logmsg + "WARNING: ";
  else if(level == 3 || level == 4) logmsg = logmsg + "ERROR: "; //level 4 is used for ERROR messages when MQTT is down
  else logmsg = logmsg + "MISC: ";
  logmsg = logmsg + msg;
  Serial.println(logmsg);
  /*Add both the level of the syslog line to the end of the line, and a flag if it has been pushed (1) or not (0).*/
  logmsg += ", " + String(level);
  if(level > 0 && level < 4){
    if(_mqtt_en){
      if(!mqttClientError && !mqttHostError){
        DynamicJsonDocument doc(1024);
        doc["friendly_name"] = "System log";
        doc["value"] = logmsg;
        doc["entity"] = apSSID;
        doc["sensorId"] = "syslog";
        doc["timestamp"] = dtimestamp;
        //String dtopic = "plan-d/" + String(apSSID) + "/sys/syslog"; 
        String dtopic = "sys/devices/" + String(apSSID) + "/syslog"; 
        String jsonOutput;
        serializeJson(doc, jsonOutput);
        if(pubMqtt(dtopic, jsonOutput, false)) logmsg += ", 1";
        else logmsg += ", 0";
      }
      else logmsg += ", 0";
    }
    else logmsg += ", 1";
  }
  else if(level > 3) logmsg += ", 0";
  else logmsg += ", 1";
  if(level > 0 && spiffsMounted){
    if(sizeFile(SPIFFS, "/syslog.txt") > 5120){
      Serial.println("Swapping logfiles");
      deleteFile(SPIFFS, "/syslog0.txt");
      renameFile(SPIFFS, "/syslog.txt", "/syslog0.txt");
    }
    logmsg = logmsg + "\r\n";
    appendFile(SPIFFS, "/syslog.txt", logmsg.c_str());
  }
}

void saveResetReason(String rReason){
  if(timeSet){
    _last_reset = printLocalTime(false) + " ";
  }
  else{
    _last_reset = "";
  }
  _last_reset = _last_reset + rReason;
}

void pushSyslog(int numLines) {
  /*This function reads the last numLines of the syslog.txt file and goes over them in reverse (chronological) order.
   * If the line ends with a '0', it means the line was meant to be pushed over MQTT to the /syslog topic, but failed to be delivered to the broker.
   * If so, this function pushes the line to the /syslog topic, and because lines are pushed chronologically it completes the syslog file on the broker.
   * If a line has been succesfylly pushed, the line ending of '0' is changed in-place to '1', so the line isn't pushed again.
   */
    Serial.println(" ----Printing missing syslog lines");  
    if (!spiffsMounted) {
        Serial.println("SPIFFS is not mounted.");
        return;
    }
    File file = SPIFFS.open("/syslog.txt", "r+");
    if (!file) {
        Serial.println("Failed to open syslog.txt for reading.");
        return;
    }
    size_t len = file.size();
    size_t pos = len > 512 ? len - 512 : 0; // Start position for the last 512 bytes (this might need adjustment based on average line length)
    file.seek(pos);
    String lineBuf;
    int lineCount = 0;
    size_t zeroPos = 0; // Position of the '0' character in the file
    while (file.available()) {
        char c = file.read();
        if (c == '\n') {
            if (lineCount >= numLines) break; // Stop after reading the specified number of lines
            // Check if the line ends with a 0
            char lastChar = lineBuf.charAt(lineBuf.length() - 2);
            if (lastChar == '0') {
                // Extracting the log message without the trailing ", 0"
                String logmsg = lineBuf.substring(0, lineBuf.length() - 3);
                Serial.println(logmsg);
                if (_mqtt_en && !mqttClientError && !mqttHostError) {
                    DynamicJsonDocument doc(1024);
                    doc["friendly_name"] = "System log";
                    doc["value"] = logmsg;
                    doc["entity"] = apSSID;
                    doc["sensorId"] = "syslog";
                    doc["timestamp"] = printUnixTime(); // Assuming this function returns the current timestamp
                    String dtopic = "plan-d/" + String(apSSID) + "/sys/syslog";
                    String jsonOutput;
                    serializeJson(doc, jsonOutput);
                    if (pubMqtt(dtopic, jsonOutput, false)) {
                        // Seek back to the position of the '0' and replace it with '1'
                        file.seek(zeroPos);
                        file.write('1');
                        file.flush(); // Ensure the change is written to the file
                    }
                }
            }
            lineBuf = ""; // Clear the buffer
            lineCount++;
        } else {
            if (c == '0' && lineBuf.endsWith(", ")) {
                zeroPos = file.position() - 1; // Update the position of the '0' character
            }
            lineBuf += c;
        }
    }
    file.close();
    Serial.println(" ----End");
}
