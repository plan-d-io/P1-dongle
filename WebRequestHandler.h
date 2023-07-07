#include "SPIFFS.h"
extern bool findInConfig(String, int&, int&), processConfigJson(String, String&, bool), processConfigString(String, String&, bool), storeConfigVar(String, int, int);
extern String returnConfigVar(String, int, int, bool), returnConfig(), returnSvg();
class WebRequestHandler : public AsyncWebHandler {
public:
  WebRequestHandler() {}
  virtual ~WebRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request);

  void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);

  void handleRequest(AsyncWebServerRequest *request);
};

bool WebRequestHandler::canHandle(AsyncWebServerRequest *request){
  /*Add custom headers here with request->addInterestingHeader("ANY");*/
  return true;
}

void WebRequestHandler::handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
  /*Handler for POST and PUT requests (with a body)*/
  if(request->method() == 2 || request->method() == 4){
    Serial.print("POST/PUT to ");
    Serial.println(request->url());
    if(len > 1){
      if(request->url() == "/config" || request->url() == "/config/"){
        /*Request to update the configuration. First check if the request is in JSON format (default)*/
        String jsonResponse;
        if(processConfigJson((const char*)data, jsonResponse, true)){
          request->send(200, "text/plain", jsonResponse);
        }
        /*If not, check if it is a configuration string (e.g. form POST).*/
        else{
          String configResponse;
          String safeString = (const char*)data;
          safeString = safeString.substring(0, total);
          processConfigString(safeString, configResponse, true);
          if(configResponse != "") request->send(200, "text/plain", configResponse);
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
    Serial.print("GET to ");
    Serial.println(request->url());
    int params = request->params();
    if(request->url() == "/config" || request->url() == "/config/"){
      /*Request to query or update the configuration. First check if the request has arguments corresponding to NVS key names*/
      if(params == 0){
        /*If not, return the full configuration as JSON*/
        request->send(200, "text/plain", returnConfig()); 
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
            foundInConfig = returnConfigVar(p->name().c_str(), retVarType, retVarNum, true);
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
          request->send(200, "text/plain", response);
        }
        else request->send(404, "text/plain");
      }
    }
    else if(request->url() == "/svg"){
      request->send(200, "image/svg+xml", returnSvg());
    }
    else if(request->url() == "/cloud"){
      request->send(SPIFFS, "/cloud.html", "text/html");
    }
    else{
      request->send(SPIFFS, "/index.html", "text/html");
    }
  }
}
