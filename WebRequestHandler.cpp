#include "WebRequestHandler.h"

bool WebRequestHandler::canHandle(AsyncWebServerRequest *request)
{
  return true;
}

void WebRequestHandler::handleRequest(AsyncWebServerRequest *request)
{
  extern String jsonData, ssidList, wifi_ssid, wifi_password, mqtt_host, mqtt_id, mqtt_user, mqtt_pass, eid_webhook, last_reset, pls_unit1, pls_unit2, 
  dmPowIn, dmPowCon, dmTotCont1, dmTotCont2, dmTotInt1, dmTotInt2, dmActiveTariff, dmVoltagel1, dmVoltagel2, dmVoltagel3, dmCurrentl1, dmCurrentl2, dmCurrentl3, dmGas, dmText, dmAvDem, dmMaxDemM;
  extern int counter, mqtt_port, trigger_type, trigger_interval, pls_type1, pls_type2, pls_multi1, pls_multi2, pls_mind1, pls_mind2, pls_emuchan;
  extern unsigned long upload_throttle;
  extern char apSSID[];
  const char fallback_html[] PROGMEM = "<!DOCTYPE html><html><head><meta charset='utf-8'><meta name=\"viewport\" content=\"width=device-width,initial-scale=1,user-scalable=no\"/><title>Digital meter - WiFi settings</title><style>html {  font-family: Helvetica;  display: inline-block;  margin: 0px auto;  text-align: center;}div,fieldset,input,select {padding: 5px;font-size: 1em;}p {margin: 0.5em 0;}input {width: 100%;box-sizing: border-box;-webkit-box-sizing: border-box;-moz-box-sizing: border-box;background: #dddddd;color: #000000;}select {width: 100%;background: #dddddd;color: #000000;}body {text-align: center;font-family: verdana, sans-serif;background: #252525;}td {padding: 0px;}button {border: 0;border-radius: 0.3rem;background: #1fa3ec;color: #faffff;line-height: 2.4rem;font-size: 1.2rem;width: 100%;-webkit-transition-duration: 0.4s;transition-duration: 0.4s;cursor: pointer;}button:hover {background: #0e70a4;}.bred {background: #d43535;}.bred:hover {background: #931f1f;}.bgrn {background: #47c266;}.bgrn:hover {background: #5aaf6f;}a {color: #1fa3ec;text-decoration: none;}.p {float: left;text-align: left;}.q {float: right;text-align: right;}.r {border-radius: 0.3em;padding: 2px;margin: 6px 2px;}</style></head><script>setTimeout(function() {  getSaveState();  getInfoMessage();}, 500); function start(){    getSSIDlist();}function getSSIDlist() {  var xhttp = new XMLHttpRequest();  xhttp.onload = function() {    if (this.status == 200) {      document.getElementById(\"StateValue\").innerHTML =      this.responseText;    }  };  xhttp.open(\"GET\", \"ssidlist\", true);  xhttp.send();}function mouseoverPass(obj) {  var obj = document.getElementById('myPassword');  obj.type = \"text\";}function mouseoutPass(obj) {  var obj = document.getElementById('myPassword');  obj.type = \"password\";}function getSaveState() {  var xhttp = new XMLHttpRequest();  xhttp.onload = function() {    if (this.status == 200) {      document.getElementById(\"SaveMessage\").innerHTML =      this.responseText;    }  };  xhttp.open(\"GET\", \"settingssaved\", true);  xhttp.send();}function getInfoMessage() {  var xhttp = new XMLHttpRequest();  xhttp.onload = function() {    if (this.status == 200) {      document.getElementById(\"InfoMessage\").innerHTML =      this.responseText;    }  };  xhttp.open(\"GET\", \"info\", true);  xhttp.send();}</script><body onload=\"start()\"><div style='text-align:left;display:inline-block;color:#eaeaea;min-width:340px;'><div style='text-align:center;color:#eaeaea;'><noscript>To use the digital meter dongle, please enable JavaScript<br></noscript><h2>Digital meter dongle</h2><h3>Wifi configuration</h3><h3><span id=\"InfoMessage\" style='text-align:center;color:red;' ></span></h3></div><p><form method=\"get\" action=\"setap\"><label>SSID:</label></p><p id = \"StateValue\">STATE</p><p>Password:</p><p style=\"vertical-align: middle;\"><input name=\"pass\" length=64 type=\"password\" id=\"myPassword\" style=\"width:88%\"></p><p style=\"vertical-align: middle;\"><span title=\"eye\" onmouseover=\"mouseoverPass();\" onmouseout=\"mouseoutPass();\"><u>Show password</u></span></p><p><input type=\"submit\" class='button bgrn'></form></p><p><div style='text-align:center;color:green;' id = \"SaveMessage\" class=\"save\"><h3></h3></div></p><p><form action='/wificn' method='get'><input type=\"hidden\" name=\"rescan\" value=\"true\" /> <button>Rescan wifi</button></form></p><p><div></div></p><div style='text-align:right;font-size:11px;'><hr/><a id=\"footer\" href='https://plan-d.io' target='_blank' style='color:#aaa;'>Digital meter dongle by plan-d.io</a></div></div></body></html>";
  extern boolean wifiError, mqttHostError, mqttClientError, httpsError, wifiSTA, wifiSave, wifiScan, configSaved, rebootReq, rebootInit, 
  mqttSave, mqtt_en, mqtt_auth, mqtt_tls, updateAvailable, update_start, mTimeFound, meterError, eid_en, eidSave, eidError, ha_en, haSave,
  update_autoCheck, update_auto, pls_en, pls_emu, dev_fleet, alpha_fleet, spiffsMounted;
  if(request->url() == "/"){
    if(spiffsMounted) request->send(SPIFFS, "/index.html", "text/html");
    else request->send_P(200, "text/html", fallback_html);
  }
  else if(request->url() == "/hostname"){
    request->send(200, "text/plain", apSSID);
  }
  else if(request->url() == "/data"){
    request->send(200, "application/json", jsonData);
  }
  else if(request->url() == "/wifi"){
    if(WiFi.status() == WL_CONNECTED){
      int wifiStrenght = abs(WiFi.RSSI());
      if(wifiStrenght <= 45) request->send(SPIFFS, "/wifi-strength-4.png", "image/png");
      else if(wifiStrenght > 45 && wifiStrenght <= 55) request->send(SPIFFS, "/wifi-strength-3.png", "image/png");
      else if(wifiStrenght > 55 && wifiStrenght <= 65) request->send(SPIFFS, "/wifi-strength-2.png", "image/png");
      else if(wifiStrenght > 65 && wifiStrenght <= 75) request->send(SPIFFS, "/wifi-strength-1.png", "image/png");
      else request->send(SPIFFS, "/wifi-strength-outline.png", "image/png");
    }
    else request->send(SPIFFS, "/wifi-strength-off-outline.png", "image/png");

  }
  else if(request->url() == "/cloud"){
  if(mqtt_en){
    if(wifiSTA && !mqttHostError && !mqttClientError && !httpsError) request->send(SPIFFS, "/cloud-outline.png", "image/png");
    else request->send(SPIFFS, "/cloud-alert.png", "image/png");
  }
  else request->send(SPIFFS, "/cloud-off-outline.png", "image/png");
    
  }
  else if(request->url() == "/meter"){
    if(mTimeFound) request->send(SPIFFS, "/counter.png", "image/png");
    else if (mTimeFound && meterError) request->send(SPIFFS, "/counter-warning.png", "image/png");
    else request->send(SPIFFS, "/counter-off.png", "image/png");
    
  }
  else if(request->url() == "/cs"){
    request->send(SPIFFS, "/syslog.html", "text/html");
  }
  else if(request->url() == "/sensor"){
    request->send(SPIFFS, "/connection-off.png", "image/png");
  }
  else if(request->url() == "/eye"){
    request->send(SPIFFS, "/eye.png", "image/png");
  }
  else if(request->url() == "/config"){
    request->send(SPIFFS, "/config.html", "text/html");
  }
  else if(request->url() == "/io"){
    request->send(SPIFFS, "/io.html", "text/html");
  }
  else if(request->url() == "/wificn"){
    configSaved = false;
    if(request->hasParam("rescan")){
      //scanWifi();
      wifiScan = true;
    }
    if(spiffsMounted) request->send(SPIFFS, "/wifi.html", "text/html");
    else request->send_P(200, "text/html", fallback_html);
  }
  else if(request->url() == "/cloudcn"){
    configSaved = false;
    request->send(SPIFFS, "/cloud.html", "text/html");
  }
  else if(request->url() == "/setap"){
    String temp_wifi_ssid, temp_pass;
    int params = request->params();
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isPost()){
      } else {
        if(p->name() == "ssid"){
          temp_wifi_ssid = p->value();
        }
        else if(p->name() == "pass"){
          temp_pass = p->value();
          if(temp_pass.length() == 0) temp_pass = wifi_password;
        }
      }
    }
    if(temp_wifi_ssid.length() > 0){
      wifi_ssid = temp_wifi_ssid;  
      wifi_password = temp_pass;
      wifiSave = true;
      if(saveConfig()){
        Serial.println("Saved wifi settings");
        configSaved = true;
        rebootReq = true;
      }
      //scanWifi();    
    }
    if(spiffsMounted) request->send(SPIFFS, "/wifi.html", "text/html");
    else request->send_P(200, "text/html", fallback_html);
  }
  else if(request->url() == "/setcloud"){
    configSaved = false;
    int params = request->params();
    if(request->hasParam("mqtt_en")){
      AsyncWebParameter* p = request->getParam("mqtt_en");
      if(p->value() == "true") mqttSave = true;
    }
    else{
      mqtt_en = false;
      mqttSave = false;
    }
    if(request->hasParam("mqtt_auth")){
      AsyncWebParameter* p = request->getParam("mqtt_auth");
      if(p->value() == "true") mqtt_auth = true;
    }
    else mqtt_auth = false;
    if(request->hasParam("mqtt_tls")){
      AsyncWebParameter* p = request->getParam("mqtt_tls");
      if(p->value() == "true") mqtt_tls = true;
    }
    else mqtt_tls = false;
    if(request->hasParam("eid_en")){
      AsyncWebParameter* p = request->getParam("eid_en");
      if(p->value() == "true") eidSave = true;
    }
    else{
      eid_en = false;
      eidSave = false;
    }
    if(request->hasParam("ha_en")){
      AsyncWebParameter* p = request->getParam("ha_en");
      if(p->value() == "true") haSave = true;
    }
    else{
      ha_en = false;
      haSave = false;
    }
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      //Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
      if(p->name() == "mqtt_host"){
        mqtt_host = p->value();
      }
      else if(p->name() == "mqtt_port"){
        mqtt_port = p->value().toInt();
      }
      else if(p->name() == "mqtt_id"){
        mqtt_id = p->value();
      }
      else if(p->name() == "mqtt_user"){
        mqtt_user = p->value();
      }
      else if(p->name() == "mqtt_pass"){
        mqtt_pass = p->value();
      }
      else if(p->name() == "upload_throttle"){
        upload_throttle = atol(p->value().c_str());
      }
      else if(p->name() == "eid_webhook"){
        eid_webhook = p->value();
      }
    }
    if(saveConfig()){
      configSaved = true;
      rebootReq = true;
    }
    request->send(SPIFFS, "/cloud.html", "text/html");
  }
  else if(request->url() == "/ssidlist"){
    request->send(200, "text/plain", ssidList);
  }
  else if(request->url() == "/settingssaved"){
    String save;
    if(configSaved) save = "Settings saved";    
    request->send(200, "text/plain", save);
  }
  else if(request->url() == "/info"){
    String rbr;
    if(wifiError) rbr = rbr + "Could not connect to<br>" + wifi_ssid + "<br>";
    if(rebootReq) rbr = rbr + "Please restart the device<br>to have changes take effect" + "<br>";
    if(updateAvailable) rbr = rbr + "Firmware upgrade available!<br>";
    if(httpsError) rbr = rbr + "Could not initiate TLS/SSL client<br>";
    if(mqttHostError) rbr = rbr + "Could not resolve MQTT broker<br> " + mqtt_host + ":<br>invalid IP/hostname<br>";
    if(mqttClientError) rbr = rbr + "Could not connect to MQTT broker,<br>" + "please check credentials<br>";
    if(eidError) rbr = rbr + "Could not connect to EnergieID,<br>" + "please check credentials<br>";
    //need to reform this to a 'information' endpoint    
    request->send(200, "text/plain", rbr);
  }
  else if(request->url() == "/configData"){
    request->send(200, "text/plain", getConfig());
  }
  else if(request->url() == "/indexData"){
    request->send(200, "text/plain", getIndexData());
  }
  else if(request->url() == "/indexStatic"){
    request->send(200, "text/plain", getIndexStatic());
  }
  else if(request->url() == "/reboot"){
    last_reset = "Reboot requested by user from webmin";
    if(saveConfig()){
      syslog("Reboot requested by user from webmin", 2);
      setReboot();
      request->send(SPIFFS, "/reboot.html", "text/html");
    }
    else request->send(SPIFFS, "/index.html", "text/html");
  }
  else if(request->url() == "/upgrade"){
    if(updateAvailable){
      update_start = true;
      syslog("Restarting for update", 2);
      last_reset = "Restarting for update";
      if(saveConfig()){
        request->send(SPIFFS, "/index.html", "text/html");
        delay(200);
        ESP.restart();
      }
      else request->send(SPIFFS, "/index.html", "text/html");
    }
    else request->send(SPIFFS, "/index.html", "text/html");
  }
  else if(request->url() == "/syslog"){
    request->send(SPIFFS, "/syslog.txt", "text/plain");
  }
  else if(request->url() == "/syslog0"){
    request->send(SPIFFS, "/syslog0.txt", "text/plain");
  }
  else if(request->url() == "/scripts.js"){
    request->send(SPIFFS, "/scripts.js", "text/javascript");
  }
  else if(request->url() == "/style.css"){
    request->send(SPIFFS, "/style.css", "text/css");
  }
  else if(request->url() == "/unitData"){
    request->send(200, "text/plain", getUnit());
  }
  else if(request->url() == "/ioData"){
    request->send(200, "text/plain", getIo());
  }
  else if(request->url() == "/unit"){
    configSaved = false;
    request->send(SPIFFS, "/unit.html", "text/html");
  }
  else if(request->url() == "/configdm"){
    configSaved = false;
    request->send(SPIFFS, "/dm.html", "text/html");
  }
  else if(request->url() == "/dmData"){
    request->send(200, "text/plain", getDm());
  }
  else if(request->url() == "/setunit"){
    //Serial.println("Got setunit");
    int params = request->params();
    if(request->hasParam("update_autoCheck")){
      AsyncWebParameter* p = request->getParam("update_autoCheck");
      if(p->value() == "true") update_autoCheck = true;
      else update_autoCheck = false;
    }
    else update_autoCheck = false;
    if(request->hasParam("update_auto")){
      AsyncWebParameter* p = request->getParam("update_auto");
      if(p->value() == "true") update_auto = true;
      else update_auto = false;
    }
    else update_auto = false;
    if(request->hasParam("alpha_fleet")){
      AsyncWebParameter* p = request->getParam("alpha_fleet");
      if(p->value() == "true") alpha_fleet = true;
      else alpha_fleet = false;
    }
    else alpha_fleet = false;
    if(request->hasParam("beta_fleet")){
      AsyncWebParameter* p = request->getParam("beta_fleet");
      if(p->value() == "true") dev_fleet = true;
      else dev_fleet = false;
    }
    else dev_fleet = false;
    if(dev_fleet && alpha_fleet){ //make sure you cant subscribe to two channels
      dev_fleet = false;
      alpha_fleet = true;
    }
    if(saveConfig()){
      configSaved = true;
      rebootReq = true;
    }
    request->send(SPIFFS, "/unit.html", "text/html");
  }
  else if(request->url() == "/setDm"){
    //Serial.println("Got setDM");
    int params = request->params();
    if(request->hasParam("trigger_type")){
      AsyncWebParameter* p = request->getParam("trigger_type");
      if(p->value() == "interval") trigger_type = 1;
      else if(p->value() == "external") trigger_type = 2;
      else trigger_type = 0;
    }
    if(request->hasParam("trigger_interval")){
      AsyncWebParameter* p = request->getParam("trigger_interval");
      trigger_interval = p->value().toInt();
    }
    if(request->hasParam("dmAvDem")){
      AsyncWebParameter* p = request->getParam("dmAvDem");
      if(p->value() == "true") dmAvDem = "1";
      else dmAvDem = "0";
    }
    else dmAvDem = "0";
    if(request->hasParam("dmMaxDemM")){
      AsyncWebParameter* p = request->getParam("dmMaxDemM");
      if(p->value() == "true") dmMaxDemM = "1";
      else dmMaxDemM = "0";
    }
    else dmMaxDemM = "0";
    if(request->hasParam("dmPowCon")){
      AsyncWebParameter* p = request->getParam("dmPowCon");
      if(p->value() == "true") dmPowCon = "1";
      else dmPowCon = "0";
    }
    else dmPowCon = "0";
    if(request->hasParam("dmPowIn")){
      AsyncWebParameter* p = request->getParam("dmPowIn");
      if(p->value() == "true") dmPowIn = "1";
      else dmPowIn = "0";
    }
    else dmPowIn = "0";
    if(request->hasParam("dmTotCont1")){
      AsyncWebParameter* p = request->getParam("dmTotCont1");
      if(p->value() == "true") dmTotCont1 = "1";
      else dmTotCont1 = "0";
    }
    else dmTotCont1 = "0";
    if(request->hasParam("dmTotCont2")){
      AsyncWebParameter* p = request->getParam("dmTotCont2");
      if(p->value() == "true") dmTotCont2 = "1";
      else dmTotCont2 = "0";
    }
    else dmTotCont2 = "0";
    if(request->hasParam("dmTotInt1")){
      AsyncWebParameter* p = request->getParam("dmTotInt1");
      if(p->value() == "true") dmTotInt1 = "1";
      else dmTotInt1 = "0";
    }
    else dmTotInt1 = "0";
    if(request->hasParam("dmTotInt2")){
      AsyncWebParameter* p = request->getParam("dmTotInt2");
      if(p->value() == "true") dmTotInt2 = "1";
      else dmTotInt2 = "0";
    }
    else dmTotInt2 = "0";
    if(request->hasParam("dmActiveTariff")){
      AsyncWebParameter* p = request->getParam("dmActiveTariff");
      if(p->value() == "true") dmActiveTariff = "1";
      else dmActiveTariff = "0";
    }
    else dmActiveTariff = "0";
    if(request->hasParam("dmVoltagel1")){
      AsyncWebParameter* p = request->getParam("dmVoltagel1");
      if(p->value() == "true") dmVoltagel1 = "1";
      else dmVoltagel1 = "0";
    }
    else dmVoltagel1 = "0";
    if(request->hasParam("dmVoltagel2")){
      AsyncWebParameter* p = request->getParam("dmVoltagel2");
      if(p->value() == "true") dmVoltagel2 = "1";
      else dmVoltagel2 = "0";
    }
    else dmVoltagel2 = "0";
    if(request->hasParam("dmVoltagel3")){
      AsyncWebParameter* p = request->getParam("dmVoltagel3");
      if(p->value() == "true") dmVoltagel3 = "1";
      else dmVoltagel3 = "0";
    }
    else dmVoltagel3 = "0";
    if(request->hasParam("dmCurrentl1")){
      AsyncWebParameter* p = request->getParam("dmCurrentl1");
      if(p->value() == "true") dmCurrentl1 = "1";
      else dmCurrentl1 = "0";
    }
    else dmCurrentl1 = "0";
    if(request->hasParam("dmCurrentl2")){
      AsyncWebParameter* p = request->getParam("dmCurrentl2");
      if(p->value() == "true") dmCurrentl2 = "1";
      else dmCurrentl2 = "0";
    }
    else dmCurrentl2 = "0";
    if(request->hasParam("dmCurrentl3")){
      AsyncWebParameter* p = request->getParam("dmCurrentl3");
      if(p->value() == "true") dmCurrentl3 = "1";
      else dmCurrentl3 = "0";
    }
    else dmCurrentl3 = "0";
    if(request->hasParam("dmGas")){
      AsyncWebParameter* p = request->getParam("dmGas");
      if(p->value() == "true") dmGas = "1";
      else dmGas = "0";
    }
    else dmGas = "0";
    if(request->hasParam("dmText")){
      AsyncWebParameter* p = request->getParam("dmText");
      if(p->value() == "true") dmText = "1";
      else dmText = "0";
    }
    else dmText = "0";
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      //Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
    }
    if(saveConfig()){
      configSaved = true;
      rebootReq = true;
    }
    request->send(SPIFFS, "/dm.html", "text/html");
  }
  else if(request->url() == "/setio"){
    configSaved = false;
    int params = request->params();
    if(request->hasParam("pls_en")){
      AsyncWebParameter* p = request->getParam("pls_en");
      if(p->value() == "true") pls_en = true;
    }
    else{
      pls_en = false;
    }
    if(request->hasParam("pls_emu")){
      AsyncWebParameter* p = request->getParam("pls_emu");
      if(p->value() == "true") pls_emu = true;
    }
    else{
      pls_en = false;
    }
    if(request->hasParam("pls_type1")){
      AsyncWebParameter* p = request->getParam("pls_type1");
      pls_type1 = p->value().toInt();
    }
    if(request->hasParam("pls_multi1")){
      AsyncWebParameter* p = request->getParam("pls_multi1");
      pls_multi1 = p->value().toInt();
    }
    if(request->hasParam("pls_unit1")){
      AsyncWebParameter* p = request->getParam("pls_unit1");
      pls_unit1 = p->value();
    }
    if(request->hasParam("pls_mind1")){
      AsyncWebParameter* p = request->getParam("pls_mind1");
      pls_mind1 = p->value().toInt();
    }
    if(request->hasParam("pls_type2")){
      AsyncWebParameter* p = request->getParam("pls_type2");
      pls_type2 = p->value().toInt();
    }
    if(request->hasParam("pls_multi2")){
      AsyncWebParameter* p = request->getParam("pls_multi2");
      pls_multi1 = p->value().toInt();
    }
    if(request->hasParam("pls_unit2")){
      AsyncWebParameter* p = request->getParam("pls_unit2");
      pls_unit2 = p->value();
    }
    if(request->hasParam("pls_mind2")){
      AsyncWebParameter* p = request->getParam("pls_mind2");
      pls_mind2 = p->value().toInt();
    }
    if(request->hasParam("pls_emuchan")){
      AsyncWebParameter* p = request->getParam("pls_emuchan");
      pls_emuchan = p->value().toInt();
    }
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      //Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
    }
    if(saveConfig()){
      configSaved = true;
      rebootReq = true;
    }
    request->send(SPIFFS, "/io.html", "text/html");
  }
  else{
    if(spiffsMounted) request->send(SPIFFS, "/index.html", "text/html");
    else request->send_P(200, "text/html", fallback_html);
  }  
}
