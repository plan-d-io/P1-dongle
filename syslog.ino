void syslog(String msg, int level){
  String logmsg;
  unsigned long dtimestamp; 
  if(timeSet){
    dtimestamp = printUnixTime();
    logmsg = printLocalTime(false) + " ";
  }
  if(level == 0 || level == 1) logmsg = logmsg + "INFO: ";
  else if(level == 2) logmsg = logmsg + "WARNING: ";
  else if(level == 3) logmsg = logmsg + "ERROR: ";
  else logmsg = logmsg + "MISC: ";
  logmsg = logmsg + msg;
  Serial.println(logmsg);
  if(mqtt_en){
    DynamicJsonDocument doc(1024);
    doc["friendly_name"] = "System log";
    doc["value"] = logmsg;
    doc["entity"] = apSSID;
    doc["sensorId"] = "syslog";
    doc["timestamp"] = dtimestamp;
    String dtopic = "sys/devices/" + String(apSSID) + "/syslog";
    String jsonOutput;
    serializeJson(doc, jsonOutput);
    pubMqtt(dtopic, jsonOutput, true);
  }
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

void appendFile(fs::FS &fs, const char * path, const char * message){
    //Serial.printf("Appending to file: %s\r\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        //Serial.println("- failed to open file for appending");
        return;
    }
    if(file.print(message)){
        //Serial.println("- message appended");
    } else {
        //Serial.println("- append failed");
    }
    file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
    //Serial.printf("Renaming file %s to %s\r\n", path1, path2);
    if (fs.rename(path1, path2)) {
        //Serial.println("- file renamed");
    } else {
        //Serial.println("- rename failed");
    }
}

void deleteFile(fs::FS &fs, const char * path){
    //Serial.printf("Deleting file: %s\r\n", path);
    if(fs.remove(path)){
        //Serial.println("- file deleted");
    } else {
        //Serial.println("- delete failed");
    }
}

int sizeFile(fs::FS &fs, const char * path){
    //Serial.printf("Getting size of file: %s\r\n", path);
    int fileSize = 0;
    File file = fs.open(path);
    if(!file || file.isDirectory()){
        //Serial.println("- failed to open file for reading");
        return fileSize;
    }
    //Serial.println("- read from file:");
    //Serial.print("File size is ");
    fileSize = file.size();
    //Serial.print(file.size());
    //Serial.println(" bytes");
    file.close();
    return fileSize;
}
