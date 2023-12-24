/*The configuration module stores and retrieves key,value pairs from non-volatile (NVS) storage,
  processes configuration strings/JSON coming in through the API or MQTT, and generates JSON response strings.*/
  
boolean restoreConfig(){
  /*Open the NVS in read-only mode, then loop through all the configuration data stores, checking if the configured NVS key name
   * is present in the NVS. If so, load the associated value from NVS and assign it to configured variable through its pointer.
   * When all stores have been checked, close the NVS again to prevent corruption.
   */
  preferences.begin("cofy-config", true);
  for(int i = 0; i < sizeof(configBool)/sizeof(configBool[0]); i++){
    if(preferences.isKey(configBool[i].configName.c_str())) *configBool[i].var = preferences.getBool(configBool[i].configName.c_str());
    else *configBool[i].var = configBool[i].defaultValue;
  }
  for(int i = 0; i < sizeof(configInt)/sizeof(configInt[0]); i++){
    if(preferences.isKey(configInt[i].configName.c_str())) *configInt[i].var = preferences.getInt(configInt[i].configName.c_str());
    else *configInt[i].var = configInt[i].defaultValue;
  }
  for(int i = 0; i < sizeof(configUInt)/sizeof(configUInt[0]); i++){
    if(preferences.isKey(configUInt[i].configName.c_str())) *configUInt[i].var = preferences.getUInt(configUInt[i].configName.c_str());
    else *configUInt[i].var = configUInt[i].defaultValue;
  }
  for(int i = 0; i < sizeof(configULong)/sizeof(configULong[0]); i++){
    if(preferences.isKey(configULong[i].configName.c_str())) *configULong[i].var = preferences.getULong(configULong[i].configName.c_str());
    else *configULong[i].var = configULong[i].defaultValue;
  }
  for(int i = 0; i < sizeof(configString)/sizeof(configString[0]); i++){
    if(preferences.isKey(configString[i].configName.c_str())) *configString[i].var = preferences.getString(configString[i].configName.c_str());
    else *configString[i].var = configString[i].defaultValue;
  }
  for(int i = 0; i < sizeof(configPass)/sizeof(configPass[0]); i++){
    if(preferences.isKey(configPass[i].configName.c_str())) *configPass[i].var = preferences.getString(configPass[i].configName.c_str());
    else *configPass[i].var = configPass[i].defaultValue;
  }
  for(int i = 0; i < sizeof(configSecret)/sizeof(configSecret[0]); i++){
    if(preferences.isKey(configSecret[i].configName.c_str())) *configSecret[i].var = preferences.getString(configSecret[i].configName.c_str());
    else *configSecret[i].var = configSecret[i].defaultValue;
  }
  for(int i = 0; i < sizeof(configIP)/sizeof(configIP[0]); i++){
    if(preferences.isKey(configIP[i].configName.c_str())) *configIP[i].var = preferences.getUInt(configIP[i].configName.c_str());
    else *configIP[i].var = configIP[i].defaultValue;
  }
  preferences.end();
  /*Temp bootstrap for pre-V2.0 compatibility*/
  if(_dev_fleet) _rel_chan = "develop";
  else if(_alpha_fleet) _rel_chan = "alpha";
  else if(_v2_fleet) _rel_chan = "V2";
  else _rel_chan = "main";
  if(_mqtt_id == "") _mqtt_id = String(apSSID);
  /*End temp bootstrap*/
  if(_uuid == ""){
    syslog("No UUID found, generating new one", 1);
    byte mac[6];
    WiFi.macAddress(mac);
    uint32_t macPart = ((uint32_t)mac[2] << 24) | ((uint32_t)mac[3] << 16) | ((uint32_t)mac[4] << 8) | (uint32_t)mac[5];
    uint32_t seed2 = random(999999999);
    uuid.seed(seed2, macPart);
    uuid.generate();
    char* uuidCharArray = uuid.toCharArray();
    char newArray[9]; 
    memcpy(newArray, uuidCharArray, 8);
    newArray[8] = '\0';
    _uuid = "P1" + String(newArray);
    syslog("Generated new UUID: " + _uuid, 1);
    _eidclaim = "";
  }
  if(_eidclaim == ""){
    _eidclaim = _uuid.substring(2);
  }
  return true;
}

boolean saveConfig(){
  /*Open the NVS in read/write mode, then loop through all the configuration data stores. Write every configured variable to NVS
   * using its configured key name. If the key,value pair is already present, it will be overwritten, if not, it will be created.
   * Close the NVS when done.
   */
  /*Temp bootstrap for pre-V2.0 compatibility*/
  if(_rel_chan == "develop") {
    _dev_fleet = true;
    _alpha_fleet = false;
    _v2_fleet = false;
  }
  else if(_rel_chan == "alpha"){
    _alpha_fleet= true;
    _dev_fleet = false;
    _v2_fleet = false;
  }
  else if(_rel_chan == "V2"){
    _v2_fleet = true;
    _alpha_fleet = false;
    _dev_fleet = false;
  }
  else if(_rel_chan == "main"){
    _v2_fleet = false;
    _alpha_fleet = false;
    _dev_fleet = false;
  }
  /*End temp bootstrap*/
  preferences.begin("cofy-config", false);
  for(int i = 0; i < sizeof(configBool)/sizeof(configBool[0]); i++){
    preferences.putBool(configBool[i].configName.c_str(), *configBool[i].var);
  }
  for(int i = 0; i < sizeof(configInt)/sizeof(configInt[0]); i++){
    preferences.putInt(configInt[i].configName.c_str(), *configInt[i].var);
  }
  for(int i = 0; i < sizeof(configUInt)/sizeof(configUInt[0]); i++){
    preferences.putUInt(configUInt[i].configName.c_str(), *configUInt[i].var);
  }
  for(int i = 0; i < sizeof(configULong)/sizeof(configULong[0]); i++){
    preferences.putULong(configULong[i].configName.c_str(), *configULong[i].var);
  }
  for(int i = 0; i < sizeof(configString)/sizeof(configString[0]); i++){
    preferences.putString(configString[i].configName.c_str(), *configString[i].var);
  }
  for(int i = 0; i < sizeof(configPass)/sizeof(configPass[0]); i++){
    preferences.putString(configPass[i].configName.c_str(), *configPass[i].var);
  }
  for(int i = 0; i < sizeof(configSecret)/sizeof(configSecret[0]); i++){
    preferences.putString(configSecret[i].configName.c_str(), *configSecret[i].var);
  }
  for(int i = 0; i < sizeof(configIP)/sizeof(configIP[0]); i++){
    preferences.putUInt(configIP[i].configName.c_str(), *configIP[i].var);
  }
  preferences.end();
  return true;
}

boolean saveBoots(){
  /*A lightweight function to store the bootcounter early on in the boot process, to debug bootloops.*/
  preferences.begin("cofy-config", false);
  preferences.putUInt("reboots", _bootcount);
  preferences.end();
  return true;
}

boolean resetConfig() {
  /*Erase the wifi configuration and put the dongle back into AP mode on next boot, 
   * or completely erase the NVS to perform a factory reset.
   */
  preferences.begin("cofy-config", false);
  preferences.remove("WIFI_SSID");
  preferences.remove("WIFI_PASSWD");
  preferences.putBool("WIFI_STA", false);
  preferences.remove("FIP_EN");  
  if(resetWifi){
    preferences.putString("LAST_RESET", "Rebooting for WiFi reset");
    syslog("WiFi credentials reset by user", 2);
  }
  else if(factoryReset){
    preferences.remove("MQTT_EN");
    preferences.remove("MQTT_TLS");
    preferences.remove("MQTT_AUTH");
    preferences.remove("PUSH_FULL");
    preferences.remove("BETA_FLT");
    preferences.remove("ALPHA_FLT");
    preferences.remove("V2_FLT");
    preferences.remove("HA_EN");
    preferences.remove("EID_EN");
    preferences.remove("PUSH_MBUS");
    preferences.remove("MQTT_PORT");
    preferences.remove("PUSH_DSMR");
    preferences.remove("UPL_THROTTLE");
    preferences.remove("UUID");
    preferences.remove("EIDCLAIM");
    preferences.remove("MQTT_HOST");
    preferences.remove("MQTT_ID");
    preferences.remove("PUSH_DSMR");
    preferences.remove("MQTT_USER");
    preferences.remove("HA_DEVICE");
    preferences.remove("REL_CHAN");
    preferences.remove("EMAIL");
    preferences.remove("MQTT_PASS");
    preferences.remove("MQTT_PFIX");
    preferences.putString("LAST_RESET", "Rebooting for factory reset");
    syslog("Factory reset by user", 2);
  }
  preferences.end();
  delay(200);
  rebootInit = true;
  return rebootInit;
}

bool findInConfig(String param, int &varType, int &varNum){
  /*Function to check if a key name (param) exists in the configuration stores. Returns true if so.
   * Passes the type of variable (varType) and location within its respective data store (varNum) by reference back to the calling function.
   * varTypes:
   *  0: bool
   *  1: signed int
   *  2: unsigned int
   *  3: unsigned long
   *  4: String
   *  5: password (String)
   */
  boolean found = false;
  for(int i = 0; i < sizeof(configBool)/sizeof(configBool[0]); i++){
    if(configBool[i].configName == param){
      found = true;
      varType = 0;
      varNum = i;
    }
  }
  if(!found){ //do this check to avoid looping through all stores if the variable has already been located
    for(int i = 0; i < sizeof(configInt)/sizeof(configInt[0]); i++){
      if(configInt[i].configName == param){
        found = true;
        varType = 1;
        varNum = i;
      }
    }
  }
  if(!found){
    for(int i = 0; i < sizeof(configUInt)/sizeof(configUInt[0]); i++){
      if(configUInt[i].configName == param){
        found = true;
        varType = 2;
        varNum = i;
      }
    }
  }
  if(!found){
    for(int i = 0; i < sizeof(configULong)/sizeof(configULong[0]); i++){
      if(configULong[i].configName == param){
        found = true;
        varType = 3;
        varNum = i;
      }
    }
  }
  if(!found){
    for(int i = 0; i < sizeof(configString)/sizeof(configString[0]); i++){
      if(configString[i].configName == param){
        found = true;
        varType = 4;
        varNum = i;
      }
    }
  }
  if(!found){
    for(int i = 0; i < sizeof(configPass)/sizeof(configPass[0]); i++){
      if(configPass[i].configName == param){
        found = true;
        varType = 5;
        varNum = i;
      }
    }
  }
  if(!found){
    for(int i = 0; i < sizeof(configSecret)/sizeof(configSecret[0]); i++){
      if(configSecret[i].configName == param){
        found = true;
        varType = 6;
        varNum = i;
      }
    }
  }
  if(!found){
    for(int i = 0; i < sizeof(configIP)/sizeof(configIP[0]); i++){
      if(configIP[i].configName == param){
        found = true;
        varType = 7;
        varNum = i;
      }
    }
  }
  return found;
}

String returnConfigVar(String varName, int varType, int varNum, int level){
  /*Return a specific configuration variable based on its type (varType) and location (varNum) within its specific data type store.
   * The variable is encapsulated in a JSON string, to be used as a response to the HTTP API or MQTT reply.
   * The level of verbosity can be set with level:
   * 0: minimal (only key:value)
   * 1: standard
   * 2: extended with additional JSON fields, see webHelp.h
   */
  String jsonOutput;
  DynamicJsonDocument doc(1024);
  if(level == 0){
    if(varType == 0) doc[varName] = *configBool[varNum].var;
    if(varType == 1) doc[varName] = *configInt[varNum].var;
    if(varType == 2) doc[varName] = *configUInt[varNum].var;
    if(varType == 3) doc[varName] = *configULong[varNum].var;
    if(varType == 4) doc[varName] = *configString[varNum].var;
    if(varType == 5) doc[varName] = *configPass[varNum].var;
    serializeJson(doc, jsonOutput);
  }
  else{
    JsonObject configVar  = doc.createNestedObject(varName);
    if(varType == 0){
      configVar["varName"] = configBool[varNum].varName;
      configVar["type"] = "bool";
      configVar["value"] = *configBool[varNum].var;
      configVar["defaultValue"] = configBool[varNum].defaultValue;
    }
    else if(varType == 1){
      configVar["varName"] = configInt[varNum].varName;
      configVar["type"] = "int32";
      configVar["value"] = *configInt[varNum].var;
      configVar["defaultValue"] = configInt[varNum].defaultValue;
    }
    else if(varType == 2){
      configVar["varName"] = configUInt[varNum].varName;
      configVar["type"] = "uint32";
      configVar["value"] = *configUInt[varNum].var;
      configVar["defaultValue"] = configUInt[varNum].defaultValue;
    }
    else if(varType == 3){
      configVar["varName"] = configULong[varNum].varName;
      configVar["type"] = "uint64";
      configVar["value"] = *configULong[varNum].var;
      configVar["defaultValue"] = configULong[varNum].defaultValue;
    }
    else if(varType == 4){
      configVar["varName"] = configString[varNum].varName;
      configVar["type"] = "string";
      configVar["value"] = *configString[varNum].var;
      configVar["defaultValue"] = configString[varNum].defaultValue;
    }
    else if(varType == 5){
      configVar["varName"] = configPass[varNum].varName;
      configVar["type"] = "password";
      configVar["value"] = *configPass[varNum].var;
    }
    else if(varType == 6){
      configVar["varName"] = configSecret[varNum].varName;
      configVar["type"] = "secret";
      configVar["value"] = *configSecret[varNum].var;
    }
    else if(varType == 7){
      configVar["varName"] = configIP[varNum].varName;
      configVar["type"] = "ipaddress";
      configVar["value"] = uint32ToIPAddress(*configIP[varNum].var);
    }
    serializeJson(doc, jsonOutput);
    if(level == 2){
      /*Add additional JSON fields through  addJson[] matrix in webHelp.h*/
      for(int i = 0; i < sizeof(addJson)/sizeof(addJson[0]); i++){
        if(addJson[i][0] == varName){
          DynamicJsonDocument addJsonFields(256);
          if(deserializeJson(addJsonFields, addJson[i][1].c_str())){
            jsonOutput = jsonOutput.substring(0, jsonOutput.length()-2);
            jsonOutput += ",";
            jsonOutput += addJson[i][1].substring(1);
            jsonOutput += "}";
          }
        }
      }
    }
  }

  return jsonOutput;
}

boolean storeConfigVar(String keyValue, int varType, int varNum){
  long retLong;
  unsigned long retULong;
  float retFloat;
  /*As the values are all Strings, we need to perform our own data type checks here*/
  if(varType == 0){
    if(keyValue == "true" || keyValue == "True" || keyValue == "1"){
      bool testVar = true;
      *configBool[varNum].var = testVar;
    }
    else if(keyValue == "false" || keyValue == "False" || keyValue == "0"){
      bool testVar = false;
      *configBool[varNum].var = testVar;
    }
  }
  else if(varType == 1){
    if(isNumeric(keyValue, retLong, retULong, retFloat)){
      if(retLong > -2146569506){
        int testVar = (int)retLong;
        *configInt[varNum].var = testVar;
      }
    }
  }
  else if(varType == 2){
    if(isNumeric(keyValue, retLong, retULong, retFloat)){
      if(retULong < 1073549248){
        unsigned int testVar = (unsigned int)retULong;
        *configUInt[varNum].var = testVar;
      }
    }
  }
  else if(varType == 3){
    if(isNumeric(keyValue, retLong, retULong, retFloat)){
      if(retULong < 1073549264){
        unsigned long testVar = retULong;
        *configULong[varNum].var = testVar;
      }
    }
  }
  else if(varType == 4){
    *configString[varNum].var = keyValue;
  }
  else if(varType == 5){
    *configPass[varNum].var = keyValue;
  }
  else if(varType == 6){
    *configSecret[varNum].var = keyValue;
  }
  else if(varType == 7){
    *configIP[varNum].var = ipStringToUint32(keyValue);
  }
  saveConfig();
  return true;
}


boolean processConfigJson(String jsonString, String &configResponse, bool updateConfig){
  /*Process a JSON string containing configuration variables coming in through the HTTP API or MQTT, 
   * and prepare a JSON response reflecting the updated value of the configuration variables.
   * The JSON string should be formatted as {"NVS key name":value}. You can chain multiple configuration variables in one string.
   * E.g. {"WIFI_STA": true,"WIFI_SSID": "test"} 
   */
  boolean isJson = false;
  DynamicJsonDocument jsonDoc(1024);
  if(DeserializationError::Ok == deserializeJson(jsonDoc, jsonString)){
    /*First check if the string is valid JSON.
     * Note that ArduinoJSON DeserializationError detection is not very robust, and malformed JSON strings might still be flagged valid.
     * The additional checks further on should however catch any invalid configuration settings.
     */
    isJson = true;
    String foundInConfig;
    JsonObject obj = jsonDoc.as<JsonObject>();
    JsonObject documentRoot = jsonDoc.as<JsonObject>();
    for (JsonPair keyValue : documentRoot) {
      /*Loop over every JSON key:value pair and check if the key name exists in one of the data stores through findInConfig().
       * This also returns the data type and location in its respective data store.
       */
      int retVarType, retVarNum;
      if(findInConfig(keyValue.key().c_str(), retVarType, retVarNum)){
        if(updateConfig){
          /*If updateConfig is false, just return the current value of the key. If true, update the value.
           * ArduinoJSON type detection is used to doublecheck the validity of the new configuration value.
           */
          if( strcmp(keyValue.key().c_str(), "WIFI_PASSWD" ) == 0 && strlen(keyValue.value().as<char*>()) == 0) {
            //Serial.println("Empty value, skipping");
            continue;
          }
          if(retVarType == 0){
            if (keyValue.value().is<bool>()){
              bool testVar = keyValue.value().as<bool>();
              *configBool[retVarNum].var = testVar;
            }
          }
          else if(retVarType == 1){
              int testVar = keyValue.value().as<signed int>();
              *configInt[retVarNum].var = testVar;
          }
          else if(retVarType == 2){
              unsigned int testVar = keyValue.value().as<unsigned int>();
              *configUInt[retVarNum].var = testVar;
          }
          else if(retVarType == 3){
              unsigned long testVar = keyValue.value().as<unsigned long>();
              *configULong[retVarNum].var = testVar;
              if(strcmp(keyValue.key().c_str(), "PUSH_DSMR") == 0){
                int shift = numKeys();
                int new_value = (testVar >> shift) & 0xFFFFFFFF;
              }
          }
          else if(retVarType == 4){
            if (keyValue.value().is<const char*>()){
              String testVar = keyValue.value().as<const char*>();
              *configString[retVarNum].var = testVar;
            }
          }
          else if(retVarType == 5){
            if (keyValue.value().is<const char*>()){
              String testVar = keyValue.value().as<const char*>();
              *configPass[retVarNum].var = testVar;
            }
          }
          else if(retVarType == 6){
            if (keyValue.value().is<const char*>()){
              String testVar = keyValue.value().as<const char*>();
              *configSecret[retVarNum].var = testVar;
            }
          }
          else if(retVarType == 7){
            if (keyValue.value().is<const char*>()){
              String testVar = keyValue.value().as<const char*>();
              *configIP[retVarNum].var = ipStringToUint32(testVar);
            }
          }
        }
        /*Build a JSON response containing the new value for every updated key, concatenate if there are multiple*/
        foundInConfig = returnConfigVar(keyValue.key().c_str(), retVarType, retVarNum, 1);
        if(foundInConfig != ""){
          configResponse += foundInConfig.substring(1, foundInConfig.length()-1);
          configResponse += ",";
        }
      }
    }
    saveConfig();
    infoMsg = "Please reboot the dongle to have changes take effect";
    mqttHostError = true;
    sinceConnCheck = 60000;
    /*Tidy up the concatenation. The response is passed by reference.*/
    if(configResponse != ""){
      configResponse = configResponse.substring(0, configResponse.length()-1);
      configResponse = "{" + configResponse;
      configResponse += "}";
    }
  }
  configBuffer = returnConfig();
  return isJson;
}

boolean processConfigString(String confString, String &response, bool updateConfig){
  /*Process a string containing configuration variables coming in through the HTTP API (or MQTT), 
   * and prepare a JSON response reflecting the updated value of the configuration variables.
   * The string should be formatted as WIFI_STA=true\r\nWIFI_SSID=test
   * This function is mainly intended for HTTP forms returning input data over POST.
    */
  String foundInConfig;
  int prevsep, separator;
  while(separator > -1){
    separator = confString.indexOf('\n');
    /*Extract individual key=value pairs*/
    String subString = confString.substring(0, separator);
    subString = subString.substring(0, separator);
    if(subString != ""){
      /*Split each pair into its key name and key value*/
      int kvsep = subString.indexOf('=');
      String key = subString.substring(0, kvsep);
      if(key == "WIFI_NW") key = "WIFI_SSID"; //temp fix for a bug in the ESPasyncWebserver library
      String keyValue;
      int sepr = subString.indexOf('\r');
      if(sepr > 0){
        keyValue = subString.substring(kvsep+1, sepr);
      }
      else keyValue = subString.substring(kvsep+1);
      int retVarType, retVarNum;
      if(findInConfig(key.c_str(), retVarType, retVarNum)){
        /*check if the key name exists in one of the data stores through findInConfig()*/
        if(updateConfig){
          storeConfigVar(keyValue.c_str(), retVarType, retVarNum);
        }
        foundInConfig = returnConfigVar(key, retVarType, retVarNum, 1);
        if(foundInConfig != ""){
          response += foundInConfig.substring(1, foundInConfig.length()-1);
          response += ",";
        }
      }
    }
    confString = confString.substring(separator+1);
    prevsep = separator;
  }
  if(response != ""){
    response = response.substring(0, response.length()-1);
    response = "{" + response;
    response += "}";
  }
  return true;
}

boolean isNumeric(String &varValue, long &longValue, unsigned long &ulongValue, float &floatValue){
  /*Helper function to determine if a String contains a numeric value, and if it is signed, unsigned or float.
   * The values are passed by reference.
   */
  unsigned int len = varValue.length()+1;
  bool isNumeric;
  if(len < 16){ //prevent overflows
    char buf[len];
    varValue.toCharArray(buf, len); //copy String to char array
    bool isInt = true;
    bool isFloat = false, foundDecimal = false, isSigned = false;
    for (int i = 0; i < len-1; i++){
      /*Loop through char array, check if every char is a digit or not*/
      if (!isDigit(buf[i])){
        /*If it is not a digit, check if it is a - (negative value) or a decimal point*/
        if(buf[i] == '-' && i == 0) isSigned = true;
        else if(buf[i] == '.' && !foundDecimal){ //there can only be one decimal point
          foundDecimal = true;
          isFloat = true;
          isInt = false;
        }
        else{
          isFloat = false;
          isInt = false;
        }
      }
    }
    isNumeric = isInt || isFloat;
    if(isInt) {
      if(isSigned) longValue = varValue.toInt();
      else {
        ulongValue = strtoul((const char*) buf, NULL, 10);
        longValue = varValue.toInt();
      }
    }
    else if(isFloat) floatValue = varValue.toFloat();
  }
  return isNumeric;
}

String returnConfig(){
  /*Return the entire NVS configuration as one single JSON string*/
  String jsonOutput;
  DynamicJsonDocument doc(5120);
  /*Runtime vars*/
  JsonObject hostVar  = doc.createNestedObject("HOSTNAME");
  hostVar["varName"] = "Dongle hostname";
  hostVar["type"] = "string";
  hostVar["value"] = String(apSSID);
  JsonObject fwVar  = doc.createNestedObject("FW_VER");
  fwVar["varName"] = "Firmware version";
  fwVar["type"] = "numeric";
  fwVar["value"] = round2(fw_ver/100.0);
  JsonObject eidintVar  = doc.createNestedObject("EID_INTV");
  eidintVar["varName"] = "Allowed upload interval";
  eidintVar["type"] = "string";
  eidintVar["value"] = eidUploadInterval;
  /*Stored config vars*/
  for(int i = 0; i < sizeof(configBool)/sizeof(configBool[0]); i++){
    if(configBool[i].includeInConfig) {
      JsonObject configVar  = doc.createNestedObject(configBool[i].configName);
      configVar["varName"] = configBool[i].varName;
      configVar["type"] = "bool";
      configVar["value"] = *configBool[i].var;
      configVar["defaultValue"] = configBool[i].defaultValue;
    }
  }
  for(int i = 0; i < sizeof(configInt)/sizeof(configInt[0]); i++){
    if(configInt[i].includeInConfig){
      JsonObject configVar  = doc.createNestedObject(configInt[i].configName);
      configVar["varName"] = configInt[i].varName;
      configVar["type"] = "int32";
      configVar["value"] = *configInt[i].var;
      configVar["defaultValue"] = configInt[i].defaultValue;
    }
  }
  for(int i = 0; i < sizeof(configUInt)/sizeof(configUInt[0]); i++){
    if(configUInt[i].includeInConfig){
      JsonObject configVar  = doc.createNestedObject(configUInt[i].configName);
      configVar["varName"] = configUInt[i].varName;
      configVar["type"] = "uint32";
      configVar["value"] = *configUInt[i].var;
      configVar["defaultValue"] = configUInt[i].defaultValue;
    }
  }
  for(int i = 0; i < sizeof(configULong)/sizeof(configULong[0]); i++){
    if(configULong[i].includeInConfig){
      JsonObject configVar  = doc.createNestedObject(configULong[i].configName);
      configVar["varName"] = configULong[i].varName;
      configVar["type"] = "uint64";
      configVar["value"] = *configULong[i].var;
      configVar["defaultValue"] = configULong[i].defaultValue;
    }
  }
  for(int i = 0; i < sizeof(configString)/sizeof(configString[0]); i++){
    if(configString[i].includeInConfig){
      JsonObject configVar  = doc.createNestedObject(configString[i].configName);
      configVar["varName"] = configString[i].varName;
      configVar["type"] = "string";
      configVar["value"] = *configString[i].var;
      configVar["defaultValue"] = configString[i].defaultValue;
    }
  }
  /*Passwords and sensitive data (e.g. GDPR stuff) are not included in the config file, only an indication if the data is present.
   * These data can still be accessed directly.
   */
  for(int i = 0; i < sizeof(configPass)/sizeof(configPass[0]); i++){
    if(configPass[i].includeInConfig){
      JsonObject configVar  = doc.createNestedObject(configPass[i].configName);
      configVar["varName"] = configPass[i].varName;
      configVar["type"] = "password";
      if(*configPass[i].var != "") configVar["filled"] = true;
    }
  }
  for(int i = 0; i < sizeof(configSecret)/sizeof(configSecret[0]); i++){
    if(configSecret[i].includeInConfig){
      JsonObject configVar  = doc.createNestedObject(configSecret[i].configName);
      configVar["varName"] = configSecret[i].varName;
      configVar["type"] = "secret";
      if(*configSecret[i].var != "") configVar["filled"] = true;
    }
  }
  for(int i = 0; i < sizeof(configIP)/sizeof(configIP[0]); i++){
    if(configIP[i].includeInConfig){
      JsonObject configVar  = doc.createNestedObject(configIP[i].configName);
      configVar["varName"] = configIP[i].varName;
      configVar["type"] = "ipaddress";
      configVar["value"] = uint32ToIPAddress(*configIP[i].var);
      configVar["defaultValue"] = String(uint32ToIPAddress(configIP[i].defaultValue));
    }
  }
  serializeJson(doc, jsonOutput);
  return jsonOutput;
}

String returnBasicConfig(){
  /*Return a JSON string containing some basic config settings, useful to send over MQTT*/
  String basicParameters[] = {"REL_CHAN", "reboots", "UPD_AUTO", "UPD_AUTOCHK", "RLT_EN", "RLT_THROTTLE", "EMAIL", "WIFI_SSID", "MQTT_HOST", "MQTT_PORT", "MQTT_ID", "MQTT_USER", "MQTT_PFIX", "UUID"};
  String response = "{\"HOSTNAME\":\"" + String(apSSID) + "\",";
  response += "\"FW_VER\":\"" + String(round2(fw_ver/100.0)) + "\",";
  for(int i = 0; i < sizeof(basicParameters)/sizeof(basicParameters[0]); i++){
    String foundInConfig;
    int retVarType, retVarNum;
    /*Check if the NVS key name exists*/ 
    if(findInConfig(basicParameters[i], retVarType, retVarNum)){
      /*Build a JSON response containing the new value for every updated key, concatenate if there are multiple*/
      foundInConfig = returnConfigVar(basicParameters[i], retVarType, retVarNum, 0);
      if(foundInConfig != ""){
        response += foundInConfig.substring(1, foundInConfig.length()-1);
        response += ",";
      }
    }
  }
  /*Tidy up the concatenation*/
  if(response != ""){
    response = response.substring(0, response.length()-1);
    response += "}";
  }
  return response;
}
