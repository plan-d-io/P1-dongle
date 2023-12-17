bool meterError, httpsError;
bool mqttHostError = true;
bool mqttClientError = true;

static const String addJson[][2] PROGMEM = {
  /*You can add custom JSON fields to individual NVS config keys here, which will be added to the JSON response when querying/updating the variables through the HTTP API or MQTT.
   * Format: { "NVS key name", "JSON string" }
   * Make sure the JSON string is valid and properly escaped!
   */
  {"WIFI_STA", "{ \"array\": [ 1, 2, 3 ], \"boolean\": true, \"color\": \"gold\", \"null\": null, \"number\": 123, \"object\": { \"a\": \"b\", \"c\": \"d\" }, \"string\": \"Hello World\" }"}, 
  {"MQTT_EN", "test"}
};

static const String svgIcons[][3] PROGMEM = {
  {"wifi-strength-4", "WiFi excellent", "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\"><path d=\"M12,3C7.79,3 3.7,4.41 0.38,7C4.41,12.06 7.89,16.37 12,21.5C16.08,16.42 20.24,11.24 23.65,7C20.32,4.41 16.22,3 12,3Z\" /></svg>"},
  {"wifi-strength-3", "WiFi good", "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\"><title>wifi-strength-3</title><path class=\"icons\" d=\"M12,3C7.79,3 3.7,4.41 0.38,7C4.41,12.06 7.89,16.37 12,21.5C16.08,16.42 20.24,11.24 23.65,7C20.32,4.41 16.22,3 12,3M12,5C15.07,5 18.09,5.86 20.71,7.45L18.77,9.88C17.26,9 14.88,8 12,8C9,8 6.68,9 5.21,9.84L3.27,7.44C5.91,5.85 8.93,5 12,5Z\" /></svg>"},
  {"wifi-strength-2", "WiFi acceptable", "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\"><title>wifi-strength-2</title><path d=\"M12,3C7.79,3 3.7,4.41 0.38,7C4.41,12.06 7.89,16.37 12,21.5C16.08,16.42 20.24,11.24 23.65,7C20.32,4.41 16.22,3 12,3M12,5C15.07,5 18.09,5.86 20.71,7.45L17.5,11.43C16.26,10.74 14.37,10 12,10C9.62,10 7.74,10.75 6.5,11.43L3.27,7.44C5.91,5.85 8.93,5 12,5Z\" /></svg>"},
  {"wifi-strength-1", "WiFi poor", "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\"><title>wifi-strength-1</title><path d=\"M12,3C7.79,3 3.7,4.41 0.38,7C4.41,12.06 7.89,16.37 12,21.5C16.08,16.42 20.24,11.24 23.65,7C20.32,4.41 16.22,3 12,3M12,5C15.07,5 18.09,5.86 20.71,7.45L15.61,13.81C14.5,13.28 13.25,13 12,13C10.75,13 9.5,13.28 8.39,13.8L3.27,7.44C5.91,5.85 8.93,5 12,5Z\" /></svg>"},
  {"wifi-strength-outline", "WiFi minimal", "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\"><title>wifi-strength-outline</title><path d=\"M12,3C7.79,3 3.7,4.41 0.38,7H0.36C4.24,11.83 8.13,16.66 12,21.5C15.89,16.66 19.77,11.83 23.64,7H23.65C20.32,4.41 16.22,3 12,3M12,5C15.07,5 18.09,5.86 20.71,7.45L12,18.3L3.27,7.44C5.9,5.85 8.92,5 12,5Z\" /></svg>"},
  {"wifi-cog", "WiFi not configured", "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\"><title>wifi-cog</title><path class=\"icons\" d=\"M12 12C9.97 12 8.1 12.67 6.6 13.8L4.8 11.4C6.81 9.89 9.3 9 12 9S17.19 9.89 19.2 11.4L18.74 12C17.66 12.05 16.63 12.33 15.73 12.81C14.6 12.29 13.33 12 12 12M21 9L22.8 6.6C19.79 4.34 16.05 3 12 3S4.21 4.34 1.2 6.6L3 9C5.5 7.12 8.62 6 12 6S18.5 7.12 21 9M12 15C10.65 15 9.4 15.45 8.4 16.2L12 21L12.22 20.71C12.08 20.16 12 19.59 12 19C12 17.57 12.43 16.24 13.17 15.13C12.79 15.05 12.4 15 12 15M23.8 20.4C23.9 20.4 23.9 20.5 23.8 20.6L22.8 22.3C22.7 22.4 22.6 22.4 22.5 22.4L21.3 22C21 22.2 20.8 22.3 20.5 22.5L20.3 23.8C20.3 23.9 20.2 24 20.1 24H18.1C18 24 17.9 23.9 17.8 23.8L17.6 22.5C17.3 22.4 17 22.2 16.8 22L15.6 22.5C15.5 22.5 15.4 22.5 15.3 22.4L14.3 20.7C14.2 20.6 14.3 20.5 14.4 20.4L15.5 19.6V18.6L14.4 17.8C14.3 17.7 14.3 17.6 14.3 17.5L15.3 15.8C15.4 15.7 15.5 15.7 15.6 15.7L16.8 16.2C17.1 16 17.3 15.9 17.6 15.7L17.8 14.4C17.8 14.3 17.9 14.2 18.1 14.2H20.1C20.2 14.2 20.3 14.3 20.3 14.4L20.5 15.7C20.8 15.8 21.1 16 21.4 16.2L22.6 15.7C22.7 15.7 22.9 15.7 22.9 15.8L23.9 17.5C24 17.6 23.9 17.7 23.8 17.8L22.7 18.6V19.6L23.8 20.4M20.5 19C20.5 18.2 19.8 17.5 19 17.5S17.5 18.2 17.5 19 18.2 20.5 19 20.5 20.5 19.8 20.5 19Z\" /></svg>"},
  {"cloud-cog-outline", "Cloud not configured", "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\"><title>cloud-cog-outline</title><path d=\"M12 19C12 19.34 12.03 19.67 12.08 20H6.5C5 20 3.69 19.5 2.61 18.43C1.54 17.38 1 16.09 1 14.58C1 13.28 1.39 12.12 2.17 11.1S4 9.43 5.25 9.15C5.67 7.62 6.5 6.38 7.75 5.43S10.42 4 12 4C13.95 4 15.6 4.68 16.96 6.04C18.32 7.4 19 9.05 19 11C20.15 11.13 21.1 11.63 21.86 12.5C21.92 12.55 21.96 12.63 22 12.69C21.1 12.25 20.08 12 19 12C18.31 12 17.63 12.11 17 12.29V11C17 9.62 16.5 8.44 15.54 7.46C14.56 6.5 13.38 6 12 6S9.44 6.5 8.46 7.46C7.5 8.44 7 9.62 7 11H6.5C5.53 11 4.71 11.34 4.03 12.03C3.34 12.71 3 13.53 3 14.5S3.34 16.29 4.03 17C4.71 17.66 5.53 18 6.5 18H12.08C12.03 18.33 12 18.66 12 19M23.83 20.64L22.83 22.37C22.76 22.5 22.63 22.5 22.5 22.5L21.27 22C21 22.18 20.73 22.34 20.43 22.47L20.24 23.79C20.22 23.91 20.11 24 20 24H18C17.86 24 17.76 23.91 17.74 23.79L17.55 22.47C17.24 22.35 16.96 22.18 16.7 22L15.46 22.5C15.34 22.5 15.21 22.5 15.15 22.37L14.15 20.64C14.09 20.53 14.12 20.4 14.21 20.32L15.27 19.5C15.25 19.33 15.24 19.17 15.24 19S15.25 18.67 15.27 18.5L14.21 17.68C14.11 17.6 14.09 17.47 14.15 17.36L15.15 15.63C15.22 15.5 15.35 15.5 15.46 15.5L16.7 16C16.96 15.82 17.25 15.66 17.55 15.53L17.74 14.21C17.76 14.09 17.87 14 18 14H20C20.11 14 20.22 14.09 20.23 14.21L20.42 15.53C20.73 15.65 21 15.82 21.27 16L22.5 15.5C22.63 15.5 22.76 15.5 22.82 15.63L23.82 17.36C23.88 17.47 23.85 17.6 23.76 17.68L22.7 18.5C22.73 18.67 22.74 18.83 22.74 19S22.72 19.33 22.7 19.5L23.77 20.32C23.86 20.4 23.89 20.53 23.83 20.64M20.5 19C20.5 18.17 19.82 17.5 19 17.5S17.5 18.17 17.5 19 18.16 20.5 19 20.5 20.5 19.83 20.5 19Z\" /></svg>"},
  {"cloud-lock-outline", "Cloud secured connected", "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\"><title>cloud-lock-outline</title><path d=\"M6.5 18H13V20H6.5C5 20 3.69 19.5 2.61 18.43C1.54 17.38 1 16.09 1 14.58C1 13.28 1.39 12.12 2.17 11.1S4 9.43 5.25 9.15C5.67 7.62 6.5 6.38 7.75 5.43S10.42 4 12 4C13.95 4 15.6 4.68 16.96 6.04C18.08 7.16 18.73 8.5 18.93 10C18.23 10 17.56 10.19 16.95 10.46C16.84 9.31 16.38 8.31 15.54 7.46C14.56 6.5 13.38 6 12 6S9.44 6.5 8.46 7.46C7.5 8.44 7 9.62 7 11H6.5C5.53 11 4.71 11.34 4.03 12.03C3.34 12.71 3 13.53 3 14.5S3.34 16.29 4.03 17C4.71 17.66 5.53 18 6.5 18M23 17.3V20.8C23 21.4 22.4 22 21.7 22H16.2C15.6 22 15 21.4 15 20.7V17.2C15 16.6 15.6 16 16.2 16V14.5C16.2 13.1 17.6 12 19 12S21.8 13.1 21.8 14.5V16C22.4 16 23 16.6 23 17.3M20.5 14.5C20.5 13.7 19.8 13.2 19 13.2S17.5 13.7 17.5 14.5V16H20.5V14.5Z\" /></svg>"},
  {"cloud-outline", "Cloud connected", "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\"><title>cloud-outline</title><path d=\"M6.5 20Q4.22 20 2.61 18.43 1 16.85 1 14.58 1 12.63 2.17 11.1 3.35 9.57 5.25 9.15 5.88 6.85 7.75 5.43 9.63 4 12 4 14.93 4 16.96 6.04 19 8.07 19 11 20.73 11.2 21.86 12.5 23 13.78 23 15.5 23 17.38 21.69 18.69 20.38 20 18.5 20M6.5 18H18.5Q19.55 18 20.27 17.27 21 16.55 21 15.5 21 14.45 20.27 13.73 19.55 13 18.5 13H17V11Q17 8.93 15.54 7.46 14.08 6 12 6 9.93 6 8.46 7.46 7 8.93 7 11H6.5Q5.05 11 4.03 12.03 3 13.05 3 14.5 3 15.95 4.03 17 5.05 18 6.5 18M12 12Z\" /></svg>"},
  {"cloud-off-outline", "Cloud not connected", "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\"><title>cloud-off-outline</title><path d=\"M19.8 22.6L17.15 20H6.5Q4.2 20 2.6 18.4T1 14.5Q1 12.58 2.19 11.08 3.38 9.57 5.25 9.15 5.33 8.95 5.4 8.76 5.5 8.57 5.55 8.35L1.4 4.2L2.8 2.8L21.2 21.2M6.5 18H15.15L7.1 9.95Q7.05 10.23 7.03 10.5 7 10.73 7 11H6.5Q5.05 11 4.03 12.03 3 13.05 3 14.5 3 15.95 4.03 17 5.05 18 6.5 18M11.13 14M21.6 18.75L20.15 17.35Q20.58 17 20.79 16.54 21 16.08 21 15.5 21 14.45 20.27 13.73 19.55 13 18.5 13H17V11Q17 8.93 15.54 7.46 14.08 6 12 6 11.33 6 10.7 6.16 10.07 6.33 9.5 6.68L8.05 5.23Q8.93 4.63 9.91 4.31 10.9 4 12 4 14.93 4 16.96 6.04 19 8.07 19 11 20.73 11.2 21.86 12.5 23 13.78 23 15.5 23 16.5 22.63 17.31 22.25 18.15 21.6 18.75M14.83 12.03Z\" /></svg>"},
  {"counter", "Digital meter connected", "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\"><title>counter</title><path d=\"M4,4H20A2,2 0 0,1 22,6V18A2,2 0 0,1 20,20H4A2,2 0 0,1 2,18V6A2,2 0 0,1 4,4M4,6V18H11V6H4M20,18V6H18.76C19,6.54 18.95,7.07 18.95,7.13C18.88,7.8 18.41,8.5 18.24,8.75L15.91,11.3L19.23,11.28L19.24,12.5L14.04,12.47L14,11.47C14,11.47 17.05,8.24 17.2,7.95C17.34,7.67 17.91,6 16.5,6C15.27,6.05 15.41,7.3 15.41,7.3L13.87,7.31C13.87,7.31 13.88,6.65 14.25,6H13V18H15.58L15.57,17.14L16.54,17.13C16.54,17.13 17.45,16.97 17.46,16.08C17.5,15.08 16.65,15.08 16.5,15.08C16.37,15.08 15.43,15.13 15.43,15.95H13.91C13.91,15.95 13.95,13.89 16.5,13.89C19.1,13.89 18.96,15.91 18.96,15.91C18.96,15.91 19,17.16 17.85,17.63L18.37,18H20M8.92,16H7.42V10.2L5.62,10.76V9.53L8.76,8.41H8.92V16Z\" /></svg>"},
  {"numeric-off", "Digital meter disconnected", "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\"><title>numeric-off</title><path d=\"M22.1 21.5L2.4 1.7L1.1 3L5.1 7H2V9H4V17H6V7.9L9.3 11.2C8.5 11.4 8 12.2 8 13V17H14V15.9L20.8 22.7L22.1 21.5M10 15V13H11.1L13.1 15H10M14 10.8L10.2 7H12C13.1 7 14 7.9 14 9V10.8M20 9H16V7H20C21.1 7 22 7.9 22 9V10.5C22 11.3 21.3 12 20.5 12C21.3 12 22 12.7 22 13.5V15C22 16 21.2 16.9 20.2 17L18.2 15H20V13H18V11H20V9Z\" /></svg>"},
  {"connection", "No external IO connected", "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\"><title>connection</title><path d=\"M21.4 7.5C22.2 8.3 22.2 9.6 21.4 10.3L18.6 13.1L10.8 5.3L13.6 2.5C14.4 1.7 15.7 1.7 16.4 2.5L18.2 4.3L21.2 1.3L22.6 2.7L19.6 5.7L21.4 7.5M15.6 13.3L14.2 11.9L11.4 14.7L9.3 12.6L12.1 9.8L10.7 8.4L7.9 11.2L6.4 9.8L3.6 12.6C2.8 13.4 2.8 14.7 3.6 15.4L5.4 17.2L1.4 21.2L2.8 22.6L6.8 18.6L8.6 20.4C9.4 21.2 10.7 21.2 11.4 20.4L14.2 17.6L12.8 16.2L15.6 13.3Z\" /></svg>"},
  {"lan-connect", "Connected to MQTT broker", "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\"><title>lan-connect</title><path d=\"M4,1C2.89,1 2,1.89 2,3V7C2,8.11 2.89,9 4,9H1V11H13V9H10C11.11,9 12,8.11 12,7V3C12,1.89 11.11,1 10,1H4M4,3H10V7H4V3M3,13V18L3,20H10V18H5V13H3M14,13C12.89,13 12,13.89 12,15V19C12,20.11 12.89,21 14,21H11V23H23V21H20C21.11,21 22,20.11 22,19V15C22,13.89 21.11,13 20,13H14M14,15H20V19H14V15Z\" /></svg>"},
  {"lan-pending", "Could not connect to MQTT broker", "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\"><title>lan-pending</title><path d=\"M4,1C2.89,1 2,1.89 2,3V7C2,8.11 2.89,9 4,9H1V11H13V9H10C11.11,9 12,8.11 12,7V3C12,1.89 11.11,1 10,1H4M4,3H10V7H4V3M3,12V14H5V12H3M14,13C12.89,13 12,13.89 12,15V19C12,20.11 12.89,21 14,21H11V23H23V21H20C21.11,21 22,20.11 22,19V15C22,13.89 21.11,13 20,13H14M3,15V17H5V15H3M14,15H20V19H14V15M3,18V20H5V18H3M6,18V20H8V18H6M9,18V20H11V18H9Z\" /></svg>"},
  {"lan-disconnect", "No MQTT broker configured", "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\"><title>lan-disconnect</title><path d=\"M4,1C2.89,1 2,1.89 2,3V7C2,8.11 2.89,9 4,9H1V11H13V9H10C11.11,9 12,8.11 12,7V3C12,1.89 11.11,1 10,1H4M4,3H10V7H4V3M14,13C12.89,13 12,13.89 12,15V19C12,20.11 12.89,21 14,21H11V23H23V21H20C21.11,21 22,20.11 22,19V15C22,13.89 21.11,13 20,13H14M3.88,13.46L2.46,14.88L4.59,17L2.46,19.12L3.88,20.54L6,18.41L8.12,20.54L9.54,19.12L7.41,17L9.54,14.88L8.12,13.46L6,15.59L3.88,13.46M14,15H20V19H14V15Z\" /></svg>"}
  };


String returnSvg(){
  String jsonOutput;
  DynamicJsonDocument doc(5120);
  JsonObject wifiVar  = doc.createNestedObject("wifi");
  if(WiFi.status() == WL_CONNECTED){
    int wifiStrenght = abs(WiFi.RSSI());
    if(wifiStrenght <= 45) {
      wifiVar["img"] = svgIcons[0][2];
      wifiVar["alt"] = svgIcons[0][1];
    }
    else if(wifiStrenght > 45 && wifiStrenght <= 55) {
      wifiVar["img"] = svgIcons[1][2];
      wifiVar["alt"] = svgIcons[1][1];
    }
    else if(wifiStrenght > 55 && wifiStrenght <= 65) {
      wifiVar["img"] = svgIcons[2][2];
      wifiVar["alt"] = svgIcons[2][1];
    }
    else if(wifiStrenght > 65 && wifiStrenght <= 75) {
      wifiVar["img"] = svgIcons[3][2];
      wifiVar["alt"] = svgIcons[3][1];
    }
    else {
      wifiVar["img"] = svgIcons[4][2];
      wifiVar["alt"] = svgIcons[4][1];
    }
  }
  else {
    wifiVar["img"] = svgIcons[5][2];
    wifiVar["alt"] = svgIcons[5][1];
  }
  JsonObject meterVar  = doc.createNestedObject("meter");
  if(!meterError){
    meterVar["img"] = svgIcons[10][2];
    meterVar["alt"] = svgIcons[10][1];
  }
  else{
    meterVar["img"] = svgIcons[11][2];
    meterVar["alt"] = svgIcons[11][1];
  }  
  JsonObject cloudVar  = doc.createNestedObject("cloud");
  if(_wifi_STA && EIDuploadEn && !httpsError){
    cloudVar["img"] = svgIcons[7][2];
    cloudVar["alt"] = svgIcons[7][1];
  }
  else{
    cloudVar["img"] = svgIcons[9][2];
    cloudVar["alt"] = svgIcons[9][1];
  }
  JsonObject localVar  = doc.createNestedObject("broker");
  if(_wifi_STA && !mqttHostError && !mqttClientError && !httpsError){
    localVar["img"] = svgIcons[13][2];
    localVar["alt"] = svgIcons[13][1];
  }
  else{
    localVar["img"] = svgIcons[15][2];
    localVar["alt"] = svgIcons[15][1];
  }
  serializeJson(doc, jsonOutput);
  return jsonOutput;
}

String releaseChannels(){
  /*Replace with a dynamic Jsondoc*/
  String channels;
  if(_alpha_fleet) channels = "{\"Releasechannels\":[{\"channel\":\"alpha\"},{\"channel\":\"develop\"},{\"channel\":\"main\"},{\"channel\":\"V2\"}]}"; 
  else if(_dev_fleet) channels = "{\"Releasechannels\":[{\"channel\":\"develop\"},{\"channel\":\"alpha\"},{\"channel\":\"main\"},{\"channel\":\"V2\"}]}";
  else if(_v2_fleet) channels = "{\"Releasechannels\":[{\"channel\":\"V2\"},{\"channel\":\"develop\"},{\"channel\":\"alpha\"},{\"channel\":\"main\"}]}";
  else channels = "{\"Releasechannels\":[{\"channel\":\"main\"},{\"channel\":\"develop\"},{\"channel\":\"alpha\"},{\"channel\":\"V2\"}]}";
  return channels;
}

String payloadFormat(){
  /*Replace with a dynamic Jsondoc*/
  String format = "{ \"Payload format\": [ { \"value\": 0, \"description\": \"Value only\" }, { \"value\": 1, \"description\": \"Basic JSON (value, unit)\" }, { \"value\": 2, \"description\": \"Standard JSON (value, unit, timestamp)\" }, { \"value\": 3, \"description\": \"COFY JSON (as above, including metadata)\" } ] }";
  return format;
}

/*Everything between rawliteral( ) is treated as one big string*/
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
    <link rel="stylesheet" href="style.css">
    <title>Digital meter - Main Menu</title>
</head>

<body>

    <div class="container">
        <noscript>To use the digital meter dongle, please enable JavaScript<br></noscript>
        <h2>Digital meter dongle</h2>
        <h2 id="hostnameHeader"></h2>
        <h3><span id="infoMessage" style='text-align:center;color:red;'></span></h3>
      
        <div class="svg-container">
            <img id="wifi" alt="" src="" />
            <img id="meter" alt="" src="" />
            <img id="cloud" alt="" src="" />
            <img id="broker" alt="" src="" />
        </div>
    
        <button class="collapsible" id="realTimeDataCollapsible">Real-time data</button>
        <div class="content">
            <div class="grid-container">
                <!-- Data will be populated here by JavaScript -->
            </div>
        </div>
        
        <form id="configForm">
            <button type="button" class="collapsible active">Basic settings</button>
            <div class="content" style="display: block;">
            <!-- Dropdown for WiFi Network -->
            <label for="WIFI_SSID">WiFi Network:</label>
            <select id="WIFI_SSID" name="WIFI_SSID"></select><br><br>
            <!-- Password Input -->
            <label for="WIFI_PASSWD">WiFi Password:</label>
            <input type="password" id="WIFI_PASSWD" name="WIFI_PASSWD"><br>
            <span class="show-password">Show password</span>
            <!-- Email Input -->
            <label for="EMAIL">User Email:</label>
            <input type="email" id="EMAIL" name="EMAIL"><br><br></div>

            <button type="button" class="collapsible">MQTT</button>
            <div class="content">
            <label for="MQTT_EN">Use MQTT client</label>
            <input type="checkbox" id="MQTT_EN" name="MQTT_EN">
            <label for="MQTT_HOST">Broker hostname or IP address</label>
            <input type="text" id="MQTT_HOST" name="MQTT_HOST"><br>
            <label for="MQTT_PORT">Broker port</label>
            <input type="number" id="MQTT_PORT" name="MQTT_PORT"><br>
            <label for="MQTT_TLS">Use TLS (SSL)</label>
            <input type="checkbox" id="MQTT_TLS" name="MQTT_TLS">
            <label for="MQTT_ID">Client ID</label>
            <input type="text" id="MQTT_ID" name="MQTT_ID"><br>
            <label for="MQTT_AUTH">Use MQTT auth</label>
            <input type="checkbox" id="MQTT_AUTH" name="MQTT_AUTH">
            <label for="MQTT_USER">Client username</label>
            <input type="text" id="MQTT_USER" name="MQTT_USER"><br>
            <label for="MQTT_PASS">Client password</label>
            <input type="password" id="MQTT_PASS" name="MQTT_PASS"><br>
            <label for="MQTT_PFIX">MQTT topic prefix</label>
            <input type="text" id="MQTT_PFIX" name="MQTT_PFIX"><br>
            <label for="UPL_THROTTLE">Update interval (s)</label>
            <input type="number" id="UPL_THROTTLE" name="UPL_THROTTLE"><br>
            <label for="FRMT_PYLD">Payload Format:</label>
            <select id="FRMT_PYLD" name="FRMT_PYLD"></select><br>
            <br><br>
            </div>

            <button type="button" class="collapsible">Home Assistant</button>
            <div class="content">
            <label for="HA_EN">Use Home Assistant</label>
            <input type="checkbox" id="HA_EN" name="HA_EN">
            <p style="text-align: left;">To use Home Assistant, please ensure you have an MQTT broker running on your Home Assistant server, and that you have configured it in the MQTT settings of the dongle (see above). The dongle uses the Home Assistant MQTT autodiscovery feature.</p>
            <label for="HA_DEVICE">Home Assistant device name</label>
            <input type="text" id="HA_DEVICE" name="HA_DEVICE"><br>
            <br><br>
            </div>

            <button type="button" class="collapsible">EnergieID</button>
            <div class="content">
            <label for="EID_EN">Use EnergieID</label>
            <input type="checkbox" id="EID_EN" name="EID_EN">
            <p style="text-align: left;">To use the EnergieID integration, copy or write down the claim code below, and connect the dongle to your WiFi network. You then have 24 hours to add the Plan-D integration to you EnergieID account, and claim your dongle using this claim code.</p>
            <label for="EIDCLAIM">Claim code</label>
            <input type="text" id="EIDCLAIM" name="EIDCLAIM" disabled><br>
            <label for="EID_INTV">Allowed upload interval</label>
            <input type="text" id="EID_INTV" name="EID_INTV" disabled><br>
            <label for="EID_PROVKEY">EnergieID provisioning key</label>
            <input type="password" id="EID_PROVKEY" name="EID_PROVKEY"><br>
            <label for="EID_PROVSEC">EnergieID provisioning secret</label>
            <input type="password" id="EID_PROVSEC" name="EID_PROVSEC"><br>
            <label for="resetUUID">Renew/unregister EnergieID</label>
            <input type="checkbox" id="resetUUID" name="resetUUID">
            <p style="text-align: left;">Generates a new EnergieID claim code on next boot. This will unregister this dongle from your current EnergieID account.</p>
            </div>

            <button type="button" class="collapsible">Advanced network settings</button>
            <div class="content">
            <label for="FIP_EN">Use fixed IP address</label>
            <input type="checkbox" id="FIP_EN" name="FIP_EN">
            <!-- IP Address -->
            <label for="FIPADDR">IP Address:</label>
            <input type="text" id="FIPADDR" name="FIPADDR">
            <!-- Default Gateway -->
            <label for="FDEFGTW">Default Gateway:</label>
            <input type="text" id="FDEFGTW" name="FDEFGTW">
            <!-- Subnet Mask -->
            <label for="FSUBN">Subnet Mask:</label>
            <input type="text" id="FSUBN" name="FSUBN" pattern="^([0-9]{1,3}\.){3}[0-9]{1,3}$" title="Enter a valid subnet mask" required>
            <!-- Primary DNS Server -->
            <label for="FDNS1">Primary DNS Server:</label>
            <input type="text" id="FDNS1" name="FDNS1" pattern="^([0-9]{1,3}\.){3}[0-9]{1,3}$" title="Enter a valid IP address" required>
            <!-- Secondary DNS Server -->
            <label for="FDNS2">Secondary DNS Server:</label>
            <input type="text" id="FDNS2" name="FDNS2" pattern="^([0-9]{1,3}\.){3}[0-9]{1,3}$" title="Enter a valid IP address">
            <br><br>
            </div>

            <button type="button" class="collapsible">Telegram settings</button>
            <div class="content">
            <label>Select meter readings to push over MQTT</label>
            <p style="text-align: left;">The list of all meter readings supported by the dongle. If a meter reading is not present in the meter telegram, it will not be pushed.</p>
            <!-- New div for sensor checkboxes -->
              <div id="sensorCheckboxes" class="sensor-checkboxes-container">
                  <!-- Checkboxes will be added here by JavaScript -->
              </div>
            <label for="PUSH_FULL">Push complete telegram over MQTT</label>
            <input type="checkbox" id="PUSH_FULL" name="PUSH_FULL">
            <p style="text-align: left;">Pushes the complete P1 telegram, as received from the meter, over MQTT. WARNING: your MQTT broker must be able to receive large MQTT messages.</p>
            </div>

            <button type="button" class="collapsible">Dongle settings</button>
            <div class="content">
            <!-- Dropdown for Release Channel -->
            <label for="REL_CHAN">Release Channel:</label>
            <select id="REL_CHAN" name="REL_CHAN"></select><br>
            <label for="RINT_SPIFFS">Update TLS bundle</label>
            <input type="checkbox" id="RINT_SPIFFS" name="RINT_SPIFFS">
            <p style="text-align: left;">Only use when TLS certificate bundle is outdated or not present. Dongle will reboot in recovery mode, restore the TLS bundle and reboot again.</p>
            </div>
            <input type="submit" class="submit" value="Submit">
        </form>
        <a href="/reboot.html" class="reboot">Reboot</a>
        <footer>
            <a href='https://www.plan-d.io' target='_blank' id="footerLink">Digital meter dongle by plan-d.io</a>
        </footer>
    </div>

    <script>
        // Helper function to fetch with timeout and retry
        function fetchWithTimeoutAndRetry(url, options = {}, timeout = 2000, retries = 3) {
            // Modify the headers to avoid compressed data
            options.headers = {
                ...options.headers,
                'Accept-Encoding': 'identity'
            };
            return new Promise((resolve, reject) => {
                const fetchPromise = fetch(url, options);
                const timeoutPromise = new Promise((_, reject) => 
                    setTimeout(() => reject(new Error('Request timed out')), timeout)
                );
        
                Promise.race([fetchPromise, timeoutPromise])
                    .then(resolve)
                    .catch(error => {
                        if (retries === 0) {
                            reject(error);
                        } else {
                            console.log(`Retrying... (${retries} attempts left)`);
                            resolve(fetchWithTimeoutAndRetry(url, options, timeout, retries - 1));
                        }
                    });
            });
        }
        function fetchData() {
            fetch('/data?basic')
                .then(response => response.json())
                .then(data => {
                    const gridContainer = document.querySelector('.grid-container');
                    gridContainer.innerHTML = ''; // Clear previous data
        
                    // Take only the first 6 elements
                    const firstSixItems = data.slice(0, 6);
        
                    firstSixItems.forEach(item => {
                        const gridItem = document.createElement('div');
                        gridItem.classList.add('grid-item');
        
                        // Create a div for the friendly_name
                        const friendlyNameDiv = document.createElement('div');
                        friendlyNameDiv.classList.add('friendly-name');
                        friendlyNameDiv.innerHTML = `<strong>${item.friendly_name}</strong>`;
                        gridItem.appendChild(friendlyNameDiv);
        
                        // Add value and unit
                        const valueDiv = document.createElement('div');
                        valueDiv.innerHTML = `${item.value} ${item.unit}`;
                        gridItem.appendChild(valueDiv);
        
                        gridContainer.appendChild(gridItem);
                    });
                })
                .catch(error => {
                    console.error("Error fetching data:", error);
                });
        }
        var coll = document.querySelectorAll(".collapsible:not(#realTimeDataCollapsible)");  //animate the collapsibles
        for (var i = 0; i < coll.length; i++) {
            coll[i].addEventListener("click", function() {
                this.classList.toggle("active");
                var content = this.nextElementSibling;
                if (content.style.display === "block") {
                    content.style.display = "none";
                } else {
                    content.style.display = "block";
                }
            });
        }
        var configData = {};
        document.addEventListener("DOMContentLoaded", function() {
              // Fetch SVG images
              fetchWithTimeoutAndRetry('/svg')
                .then(response => response.json())
                .then(data => {
                    const wifiImg = document.getElementById('wifi');
                    const meterImg = document.getElementById('meter');
                    const cloudImg = document.getElementById('cloud');
                    const brokerImg = document.getElementById('broker');
            
                    // Modify the SVG fill color to white
                    let wifiSvg = data.wifi.img.replace('<path', '<path fill="white"');
                    let meterSvg = data.meter.img.replace('<path', '<path fill="white"');
                    let cloudSvg = data.cloud.img.replace('<path', '<path fill="white"');
                    let brokerSvg = data.broker.img.replace('<path', '<path fill="white"');
            
                    wifiImg.src = 'data:image/svg+xml,' + encodeURIComponent(wifiSvg);
                    wifiImg.alt = data.wifi.alt;
                    wifiImg.title = data.wifi.alt;
            
                    meterImg.src = 'data:image/svg+xml,' + encodeURIComponent(meterSvg);
                    meterImg.alt = data.meter.alt;
                    meterImg.title = data.meter.alt;
            
                    cloudImg.src = 'data:image/svg+xml,' + encodeURIComponent(cloudSvg);
                    cloudImg.alt = data.cloud.alt;
                    cloudImg.title = data.cloud.alt;
            
                    brokerImg.src = 'data:image/svg+xml,' + encodeURIComponent(brokerSvg);
                    brokerImg.alt = data.broker.alt;
                    brokerImg.title = data.broker.alt;
            
                    return fetchWithTimeoutAndRetry('/wifi');
                })
                // Fetch the SSID list and populate the dropdown
                .then(response => response.json())
                .then(data => {
                    const ssidSelect = document.getElementById('WIFI_SSID');
                    data.SSIDlist.forEach(item => {
                        const option = document.createElement('option');
                        option.value = item.SSID;
                        option.textContent = item.SSID;
                        ssidSelect.appendChild(option);
                    });

                    // Fetch the release channels and populate the dropdown
                    return fetchWithTimeoutAndRetry('/releasechan');
                })
                .then(response => response.json())
                .then(data => {
                    const releaseChannelSelect = document.getElementById('REL_CHAN');
                    data.Releasechannels.forEach(item => {
                        const option = document.createElement('option');
                        option.value = item.channel;
                        option.textContent = item.channel;
                        releaseChannelSelect.appendChild(option);
                    });
                    // After populating the dropdowns, fetch the payload format data
                    return fetchWithTimeoutAndRetry('/payloadformat');
                })
                .then(response => response.json())
                .then(data => {
                    const payloadFormatSelect = document.getElementById('FRMT_PYLD');
                    data['Payload format'].forEach(item => {
                        const option = document.createElement('option');
                        option.value = item.value;
                        option.textContent = item.value + ' - ' + item.description;
                        payloadFormatSelect.appendChild(option);
                    });
                    // After populating the dropdowns, fetch the configuration data
                    return fetchWithTimeoutAndRetry('/config');
                })
                .then(response => response.json())
                .then(data => {
                    // Store the configuration data globally
                    configData = data; 
                    // Set the hostname header
                    const hostname = data.HOSTNAME.value;
                    document.getElementById('hostnameHeader').textContent = hostname;
                    // Set the footer version
                    const version = data.FW_VER.value;
                    const footerLink = document.getElementById('footerLink');
                    footerLink.textContent = `Digital meter dongle V${version} by plan-d.io`;
                    //Populate all the form fields
                    const inputs = document.querySelectorAll('input, select, textarea');
                    inputs.forEach(input => {
                        const name = input.name;
                        if (data[name]) {
                            const type = data[name].type;
                            const value = data[name].value;
                            switch (type) {
                                case 'bool':
                                    if (input.type === 'checkbox') {
                                        input.checked = value;
                                    } else {
                                        input.value = value ? 'true' : 'false';
                                    }
                                    break;
                                case 'int32':
                                case 'uint32':
                                case 'uint64':
                                case 'string':
                                    input.value = value;
                                    break;
                                case 'ipaddress':
                                    input.value = value;
                                    break;
                                case 'password':
                                    if (data[name].filled) {
                                        input.placeholder = 'Password is set';
                                    }
                                    break;
                                case 'secret':
                                    if (data[name].filled) {
                                        input.placeholder = 'Secret is set';
                                    }
                                    break;
                                case 'email':
                                    if (data[name].filled) {
                                        input.placeholder = 'Email is set';
                                    }
                                    break;
                                default:
                                    console.warn(`Unhandled type: ${type} for input: ${name}`);
                            }
                        }
                    });
                    createSensorCheckboxes();
                })
                .catch(error => {
                    console.error('Error fetching or processing data:', error);
                    document.getElementById('infoMessage').textContent = "Error loading data, please refresh the page";
                });

                // Function to create sensor checkboxes
                function createSensorCheckboxes() {
                    fetchWithTimeoutAndRetry('/data?all')
                        .then(response => response.json())
                        .then(data => {
                            const bitmask = configData.PUSH_DSMR.value;
                            const container = document.getElementById('sensorCheckboxes');
                            container.innerHTML = ''; // Clear previous checkboxes
                
                            data.forEach((sensor, index) => {
                                const isChecked = (bitmask & (1 << index)) !== 0;
                                const checkbox = createCheckbox(sensor.friendly_name, isChecked);
                                container.appendChild(checkbox);
                            });
                        })
                        .catch(error => console.error('Error fetching sensor data:', error));
                }
                
              function createCheckbox(name, isChecked) {
                  const container = document.createElement('div');
                  container.className = 'sensor-checkbox';
              
                  const sensorName = document.createElement('span');
                  sensorName.textContent = name; // Set sensor name as text
                  sensorName.style.textAlign = 'left'; // Align text to the left
              
                  const checkbox = document.createElement('input');
                  checkbox.type = 'checkbox';
                  checkbox.name = 'sensor';
                  checkbox.value = name;
                  checkbox.checked = isChecked;
              
                  container.appendChild(checkbox);
                  container.appendChild(sensorName); // Append sensor name text
                  return container;
              }

              // Real-time data collapsible
              const realTimeDataCollapsible = document.getElementById('realTimeDataCollapsible');
              const realTimeDataContent = realTimeDataCollapsible.nextElementSibling;
              let interval;
          
              realTimeDataCollapsible.addEventListener('click', function() {
                  this.classList.toggle('active');
                  if (realTimeDataContent.style.display === "block") {
                      realTimeDataContent.style.display = "none";
                      clearInterval(interval); // Stop fetching data when collapsed
                  } else {
                      realTimeDataContent.style.display = "block";
                      fetchData(); // Fetch data immediately on expand
                      interval = setInterval(fetchData, 1000); // Fetch data every second
                  }
              });
                
            // Mark password fields as changed when their value is modified
            const passwordFields = document.querySelectorAll('input[type="password"]');
            passwordFields.forEach(field => {
                field.addEventListener('input', function() {
                    this.setAttribute('data-changed', 'true');
                });
            });

            // Handle form submission
            const form = document.getElementById('configForm');
            form.addEventListener('submit', function(event) {
                event.preventDefault();

                const formData = new FormData(form);
                const jsonData = {};

                // Handle unchecked checkboxes (outside sensorCheckboxes div)
                const checkboxes = document.querySelectorAll('input[type="checkbox"]');
                checkboxes.forEach(checkbox => {
                    if (!checkbox.closest('.sensor-checkboxes-container')) {
                        jsonData[checkbox.name] = checkbox.checked;
                    }
                });

                // Handle checkboxes within the sensorCheckboxes div and translate to bitmask
                let bitmask = 0;
                const sensorCheckboxes = document.querySelectorAll('.sensor-checkboxes-container input[type="checkbox"]');
                sensorCheckboxes.forEach((checkbox, index) => {
                    jsonData[checkbox.name] = checkbox.checked;
                    // Calculate bitmask based on checkbox status
                    if (checkbox.checked) {
                        bitmask |= 1 << index;
                    }
                });
            
                // Include the PUSH_DSMR bitmask in the JSON data
                jsonData['PUSH_DSMR'] = bitmask;

                // Handle resetUUID checkbox
                const resetUUIDCheckbox = document.getElementById('resetUUID');
                if (resetUUIDCheckbox.checked) {
                    jsonData['UUID'] = ''; // Set UUID to an empty string
                }

                formData.forEach((value, key) => {
                    const inputElement = document.querySelector(`[name="${key}"]`);
                    if (inputElement.type !== 'checkbox') {
                        if (inputElement.type === 'password' || inputElement.type === 'email') {
                            if (value && (inputElement.getAttribute('data-changed') === 'true' || !inputElement.placeholder.includes('set'))) {
                                jsonData[key] = value;
                            }
                        } else {
                            jsonData[key] = value;
                        }
                    }
                });

                console.log('Submitting JSON:', JSON.stringify(jsonData));

                fetch('/config', {
                    method: 'PUT',
                    headers: {
                        'Content-Type': 'application/json'
                    },
                    body: JSON.stringify(jsonData)
                })
                .then(response => response.json())
                .then(data => {
                    console.log('Server response:', data);
                    // Fetch the info message after form submission
                    return fetch('/info');
                })
                .then(response => response.text())
                .then(message => {
                    document.getElementById('infoMessage').textContent = message;
                })
                .catch(error => {
                    console.error('Error submitting data:', error);
                });
            });
            // Show password functionality
            const passwordInput = document.getElementById('WIFI_PASSWD');
            const showPasswordText = document.querySelector('.show-password');

            showPasswordText.addEventListener('mouseover', function() {
                passwordInput.type = 'text';
            });

            showPasswordText.addEventListener('mouseout', function() {
                passwordInput.type = 'password';
            });
        });
    </script>
</body>

</html>

)rawliteral";

const char reboot_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
    <link rel="stylesheet" href="style.css">
    <title>Digital meter - Reboot</title>
</head>

<body>
    <div class="container">
            <div class="content" style="display: block;">
            <h2>The digital meter dongle is rebooting.</h3></p>
            <div class="loader"></div> 
            <h3>You can close this window.</h3></p>  
            </div>
        <footer>
            <a href='https://www.plan-d.io' target='_blank' id="footerLink">Digital meter dongle by plan-d.io</a>
        </footer>
        </div>
    </div>
  <script>
    document.addEventListener("DOMContentLoaded", function() {
      // Function to make a request to the /reboot endpoint
      function requestReboot() {
        fetch('/reboot')
          .then(response => {
            if (response.ok) {
              console.log('Successfully requested reboot.');
            } else {
              console.error('Error requesting reboot:', response.statusText);
            }
          })
          .catch(error => {
            console.error('Error fetching /reboot:', error);
          });
      }

      // Call the requestReboot function
      requestReboot();
    });
  </script>
</body>

</html>

)rawliteral";

const char css[] PROGMEM = R"rawliteral(
        body {
            font-family: Helvetica, verdana, sans-serif;
            background: #252525;
            color: #eaeaea;
            text-align: center;
        }

        h2 {
            color: #1fa3ec;
            padding: 0.5vh;
        }
        h3 {
            color: #1fa3ec;
        }
        /* Styles for the main SVG container */
        .svg-main-container {
            text-align: center; /* Center the SVG horizontally */
            margin-bottom: 20px; /* Add some space below the SVG */
        }
        
        .main-svg {
            width: 680px; /* Set the width of the SVG */
            height: auto; /* Maintain the aspect ratio */
            display: inline-block; /* Display the SVG as an inline element */
            margin: 0 auto; /* Center the SVG horizontally */
        }
        .icons {
          fill: white;
        }
        .collapsible {
            background-color: #1fa3ec;
            color: white;
            cursor: pointer;
            padding: 10px;
            width: 100%;
            border: none;
            text-align: left;
            outline: none;
            font-size: 15px;
            transition: background-color 0.4s;
        }

        .collapsible:hover {
            background-color: #0e70a4;
        }

        .collapsible:after {
            content: ' +';
            font-weight: bold;
            float: right;
        }

        .grid-container {
            display: grid;
            grid-template-columns: 1fr 1fr 1fr; /* 3 columns */
            grid-template-rows: 1fr 1fr; /* 2 rows */
            gap: 10px;
            padding: 10px;
        }
        
        .grid-item {
            display: flex;
            flex-direction: column;
            justify-content: space-between;
            height: 100%; /* Adjust as per your requirement */
        }

        .grid-item .friendly-name {
            margin-bottom: 2px; /* Adjust this value as per your preference */
        }

        .active:after {
            content: " -";
        }

        .show-password {
            cursor: pointer;
            color: #1fa3ec;
            text-decoration: underline;
        }

        .content {
            padding: 0 18px;
            display: none;
            overflow: hidden;
            background-color: #4f4f4f;
        }

        a {
            color: #1fa3ec;
            text-decoration: none;
        }

        .container {
            max-width: 600px;
            margin: 0 auto;
        }
        
        input[type="text"] {
            width: 100%;
            padding: 10px;
            margin: 10px 0;
            box-sizing: border-box;
        }
        input[type="password"] {
            width: 100%;
            padding: 10px;
            margin: 10px 0;
            box-sizing: border-box;
        }
        input[type="email"] {
            width: 100%;
            padding: 10px;
            margin: 10px 0;
            box-sizing: border-box;
        }
        input[type="number"] {
            width: 100%;
            padding: 10px;
            margin: 10px 0;
            box-sizing: border-box;
        }
        input[type="checkbox"] {
            display: block;
            margin: 10px 0;
            margin-left: 0;
            margin-right: auto;
        }

        label {
            color: #1fa3ec;
            font-weight: bold;
            font-size: 0.9em;
            display: block;
            margin-top: 3px;
            margin-bottom: 1px;
            text-align: left;
            display: flex;
            align-items: center;
        }

        select {
            width: 100%;
            padding: 10px;
            margin: 10px 0;
            box-sizing: border-box;
        }

        button.submit {
            margin-top: 20px;
        }

        .submit {
            width: 600px;
            background-color: #47c266; /* Green background */
            color: white;
            padding: 10px 20px;
            margin: 20px 0; /* Separation from other content */
            margin-bottom: 5px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-size: 1.2rem;
        }
        .submit:hover {
            background-color: #5aaf6f; /* Darker green on hover */
        }
        .reboot {
            display: inline-block; /* Make the link behave like a block element */
            width: 600px;
            box-sizing: border-box; 
            background-color: #8B0000; /* Red background */
            color: white;
            padding: 10px 20px;
            margin: 20px 0; /* Separation from other content */
            margin-top: 5px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-size: 1.2rem;
            text-align: center; /* Center the text */
            text-decoration: none; /* Remove the underline from the link */
        }
        
        .reboot:hover {
            background-color: #5aaf6f; /* Darker red on hover */
        }
        .svg-container {
            display: flex;
            justify-content: space-between;
            width: 480px;
            margin: 0 auto 40px auto; /* Center the container */
        }

        .svg-container img {
            width: 48px;
            height: 48px;
            fill: white;
        }
        footer {
            text-align: right;
        }

        .loader {
            margin: auto;
            border: 16px solid #1f1f1f; /* Light grey */
            border-top: 16px solid #3498db; /* Blue */
            border-radius: 50%;
            width: 60px;
            height: 60px;
            animation: spin 1s linear infinite;
            -webkit-animation: spin 1s linear infinite; /* Safari and Chrome */
            -moz-animation: spin 1s linear infinite; /* Firefox */
        }
        .sensor-checkboxes-container {
            display: grid;
            grid-template-columns: repeat(2, 1fr); /* Two columns */
            gap: 10px; /* Space between checkboxes */
        }        
        .sensor-checkbox {
            display: flex;
            flex-direction: column-reverse; /* Checkbox below the label */
            align-items: left; /* Center align */
        }
        @keyframes spin {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
        }
        
        @-webkit-keyframes spin { /* Safari and Chrome */
            0% { -webkit-transform: rotate(0deg); }
            100% { -webkit-transform: rotate(360deg); }
        }
        
        @-moz-keyframes spin { /* Firefox */
            0% { -moz-transform: rotate(0deg); }
            100% { -moz-transform: rotate(360deg); }
        }
        })rawliteral";

 const char test_html[] PROGMEM = {'\n'};
