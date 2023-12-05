/*The webserver client and its handlers live here*/
#include <LittleFS.h>
extern bool findInConfig(String, int&, int&), processConfigJson(String, String&, bool), processConfigString(String, String&, bool), storeConfigVar(String, int, int), httpDebug;
extern String returnConfigVar(String, int, int, int), returnConfig(), returnBasicConfig(), returnSvg(), ssidList, releaseChannels(), payloadFormat(), httpTelegramValues(String option), infoMsg, _user_email, configBuffer;
extern const char index_html[], reboot_html[], test_html[], css[];
extern char apSSID[];
extern void setReboot(), saveResetReason(String);
class WebRequestHandler : public AsyncWebHandler {
public:
  WebRequestHandler() {}
  virtual ~WebRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request);

  void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);

  void handleRequest(AsyncWebServerRequest *request);
};

bool WebRequestHandler::canHandle(AsyncWebServerRequest *request){
  /*Add custom headers here with request->addInterestingHeader("ANY");
  Serial.println("Webrequest");
  Serial.println(request->method());
  int headers = request->headers();
  int i;
  for(i=0;i<headers;i++){
    AsyncWebHeader* h = request->getHeader(i);
    Serial.printf("HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
  }*/
  return true;
}

void WebRequestHandler::handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
  /*Handler for POST and PUT requests (with a body)*/
  if(request->method() == 2 || request->method() == 4 || request->method() == 8){
    if(httpDebug){
      Serial.print("POST/PUT to ");
      Serial.println(request->url());
    }
    if(len > 1){
      if(request->url() == "/config" || request->url() == "/config/"){
        /*Request to update the configuration. First check if the request is in JSON format (default)*/
        String jsonResponse;
        if(processConfigJson((const char*)data, jsonResponse, true)){
          request->send(200, "application/json", jsonResponse);
        }
        /*If not, check if it is a configuration string (e.g. form POST).*/
        else{
          String configResponse;
          String safeString = (const char*)data;
          safeString = safeString.substring(0, total);
          processConfigString(safeString, configResponse, true);
          if(configResponse != "") request->send(200, "application/json", configResponse);
          else request->send(404, "text/plain");
        }
        //request->send(200, "text/plain", "post");
      }
    }
    else request->send(404, "text/plain");
  }
}

void WebRequestHandler::handleRequest(AsyncWebServerRequest *request){
  /*Handler for GET requests, with optional arguments*/
  if(request->method() == 1){
    if(httpDebug){
      Serial.print("GET to ");
      Serial.println(request->url());
    }
    int params = request->params();
    if(request->url() == "/config" || request->url() == "/config/"){
      /*Request to query or update the configuration. First check if the request has arguments corresponding to NVS key names*/
      if(params == 0){
        /*If not, return the full configuration as JSON*/
        //request->send(200, "application/json", returnConfig()); 
        request->send(200, "application/json", configBuffer); 
      }
      else{
        String response, foundInConfig;
        for(int i=0; i<params; i++){
          AsyncWebParameter* p = request->getParam(i);
          int retVarType, retVarNum;
          if(findInConfig(p->name().c_str(), retVarType, retVarNum)){
            /*Check if the NVS key name passed as argument exists*/ 
            if(p->value() != ""){
             /*If a value is passed as well, update the associated variable in its respective data store*/
             storeConfigVar(p->value(), retVarType, retVarNum);
            }
            /*Build a JSON response containing the new value for every updated key, concatenate if there are multiple*/
            foundInConfig = returnConfigVar(p->name().c_str(), retVarType, retVarNum, 1);
            if(foundInConfig != ""){
              response += foundInConfig.substring(1, foundInConfig.length()-1);
              response += ",";
            }
          }
        }
        /*Tidy up the concatenation*/
        if(response != ""){
          response = response.substring(0, response.length()-1);
          response = "{" + response;
          response += "}";
          request->send(200, "application/json", response);
        }
        else request->send(404, "text/plain");
      }
    }
    else if(request->url() == "/data"){
      if(params == 0) request->send(200, "application/json", httpTelegramValues(""));
      else{
        AsyncWebParameter* p = request->getParam(0);
        request->send(200, "application/json", httpTelegramValues(p->name().c_str()));
      }
    }
    else if(request->url() == "/wifi" || request->url() == "/wifi/"){
      request->send(200, "application/json", ssidList);
    }
    else if(request->url() == "/releasechan" || request->url() == "/releasechan/"){
      request->send(200, "application/json", releaseChannels());
    }
    else if(request->url() == "/payloadformat" || request->url() == "/payloadformat/"){
      request->send(200, "application/json", payloadFormat());
    }
    else if(request->url() == "/svg"){
      request->send(200, "application/json", returnSvg());
    }
    else if(request->url() == "/info"){
      request->send(200, "text/plain", infoMsg);
      //request->send(200, "text/plain", "Settings saved");
    }
    else if(request->url() == "/hostname"){
      request->send(200, "text/plain", apSSID);
    }
    else if(request->url() == "/email"){
      request->send(200, "text/plain", _user_email);
    }
    else if(request->url() == "/test" || request->url() == "/test/"){ //temp, just for SPIFFS testing
      request->send_P(200, "text/html", index_html);
    }
    else if(request->url() == "/reboot.html"){
      request->send_P(200, "text/html", reboot_html);
    }
    else if(request->url() == "/reboot"){
      request->send_P(200, "text/html", reboot_html);
      saveResetReason("Reboot requested from webmin");
      setReboot();
    }
    else if(request->url() == "/style.css"){
      request->send_P(200, "text/css", css);
    }
    else if(request->url() == "/syslog"){
      request->send(SPIFFS, "/syslog.txt", "text/plain");
    }
    else if(request->url() == "/syslog0"){
      request->send(SPIFFS, "/syslog0.txt", "text/plain");
    }
    else if(request->url() == "/favicon.ico"){
      request->send(SPIFFS, "", "text/plain");
    }
    else{
      request->send_P(200, "text/html", index_html);
    }
  }
}
