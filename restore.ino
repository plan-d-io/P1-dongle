/*  If the TLS certificate bundle is missing, the dongle can restore it from the remote code
 *  repository by using a static root certificate to connect to this repository as a fall-back.
 *  As this usually happens when for some reason the SPIFFS file storage partition has become corrupt,
 *  the dongle will also redownload the other static files and then reboot.
 *  We use github as the code repo, but you can change it your own. 
 * */

/*The public root certificate for the repository*/ 
const char* github_root_ca= \
     "-----BEGIN CERTIFICATE-----\n" \
     "MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n" \
     "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
     "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \
     "QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
     "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
     "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n" \
     "9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n" \
     "CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n" \
     "nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n" \
     "43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n" \
     "T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n" \
     "gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n" \
     "BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n" \
     "TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n" \
     "DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n" \
     "hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n" \
     "06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n" \
     "PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n" \
     "YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n" \
     "CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n" \
     "-----END CERTIFICATE-----\n";

#define TLSBUNDLE "/x509_crt_bundle.bin"

void restoreSPIFFS(){
  listDir(SPIFFS, "/", 3);
  /*Load the static cert into the https client*/
  if(client){
    syslog("Setting up fallback TLS/SSL client", 2);
    client->setCACert(github_root_ca);
  }
  else{
    syslog("Failed to setup fallback TLS/SSL client", 3);
  }
  boolean repoOK = true;
  syslog("Checking remote repository", 0);
  String baseUrl = "https://github.com/plan-d-io/P1-dongle/raw/";
  if(_dev_fleet) baseUrl += "develop/data/x509_crt_bundle.bin";
  else if(_alpha_fleet) baseUrl += "alpha/data/cert/x509_crt_bundle.bin";
  else if(_v2_fleet) baseUrl += "V2-0/data/x509_crt_bundle.bin";
  else baseUrl += "main/data/cert/x509_crt_bundle.bin";
  String fileUrl = "https://raw.githubusercontent.com/plan-d-io/P1-dongle/main/data/x509_crt_bundle.bin";//"https://github.com/plan-d-io/P1-dongle/raw/develop/data/x509_crt_bundle.bin";
  String s = "/x509_crt_bundle.bin";
  if(repoOK){
    /*Reformat the SPIFFS*/
    syslog("Formatting", 0);
    bool formatted = SPIFFS.format();
    if(formatted){
      syslog("Success formatting", 0);
    }
    else{
      syslog("Error formatting", 3);
    }
    File f;
    if(SPIFFS.exists(TLSBUNDLE)){
      Serial.println("Removing old bundle");
      SPIFFS.remove(TLSBUNDLE);
    }
    /*Next, store a file to SPIFFS*/
    syslog("Downloading cert bundle", 0);
    Serial.println(fileUrl);
    f = LittleFS.open(TLSBUNDLE, "w");
    if(f){
      if (https.begin(*client, fileUrl)) {
        int httpCode = https.GET();
        Serial.println(httpCode);
        if (httpCode > 0) {
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY || httpCode == HTTP_CODE_FOUND) {
            long contentLength = https.getSize();
            Serial.print("File size: ");
            Serial.println(contentLength);
            Serial.println("Begin download");
            size_t written = https.writeToStream(&f);
            if (written == contentLength) {
              Serial.println("Written : " + String(written) + " successfully");
            }
          }
          else{
            syslog("Could not fetch file, HTTPS code " + String(httpCode), 2);
          }
        } 
        else {
          syslog("Could not connect to repository, HTTPS code " + String(httpCode) +" " +  String(https.errorToString(httpCode)), 2);
        }
        https.end();
      }
      f.close();
    }
    else{
      syslog("Could not open cert bundle file for writing", 2);
    }
    /*Check if the cert bundle is present*/
    File file = LittleFS.open(TLSBUNDLE, "r");
    if(file && file.size() > 0){
      syslog("Cert bundle present on SPIFFS", 1);
    }
    if(!file) {
        syslog("Could not load cert bundle from SPIFFS", 3);
    }
    file.close();
    bundleLoaded = true;
    /*Download the other static files*/
    _reinit_spiffs = false;
    saveResetReason("Rebooting after SPIFFS restore");
    saveConfig();
    SPIFFS.end();
    delay(500);
    ESP.restart();
  }
  else{
    //probably should do something here
  }
}
