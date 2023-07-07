/*These bespoke structures are used to construct non-volatile (NVS) data storages for each data type*/

struct boolStore{
   String varName;
   bool* var;
   String configName;
   bool defaultValue;
};

struct intStore{
   String varName;
   int* var;
   String configName;
   int defaultValue;
};

struct uintStore{
   String varName;
   unsigned int* var;
   String configName;
   unsigned int defaultValue;
};

struct ulongStore{
   String varName;
   unsigned long* var;
   String configName;
   unsigned long defaultValue;
};

struct stringStore{
   String varName;
   String* var;
   String configName;
   String defaultValue;
};

struct floatStore{
   String varName;
   float* var;
   String configName;
   float defaultValue;
};

/*Declaration of global variables retrieved from NVS config storage, denoted by a leading _*/
/*Wifi*/
boolean _wifi_STA;
String _wifi_ssid, _wifi_password;
/*MQTT*/
boolean _mqtt_en, _mqtt_tls, _mqtt_auth;
unsigned int _mqtt_port;
String _mqtt_host, _mqtt_id, _mqtt_user, _mqtt_pass;
/*Update*/
boolean _update_auto, _update_autoCheck, _update_start, _update_finish, _dev_fleet, _alpha_fleet, _restore_finish;
unsigned long _fw_new;
/*Debug*/
unsigned int _bootcount;
String last_reset;
/*DSMR processing*/
unsigned long _upload_throttle;
unsigned int _trigger_interval, _trigger_type;
/*External services*/
boolean _ha_en, _eid_en;
/*Placeholder vars*/
int _tempInt;
float _tempFloat;

/*The configuration data stores for every data type.
 * Format: { "User-readable name", global variable name (reference), "NVS key name", default value }
 */
static const boolStore configBool[] PROGMEM = {
  {"WiFi Station mode", &_wifi_STA, "WIFI_STA", false}, 
  {"MQTT enabled", &_mqtt_en, "MQTT_EN", false},
  {"MQTT secure", &_mqtt_tls, "MQTT_TLS", false},
  {"MQTT auth", &_mqtt_auth, "MQTT_AUTH", false},
  {"Update auto", &_update_auto, "UPD_AUTO", true},
  {"Update autocheck", &_update_autoCheck, "UPD_AUTOCHK", true},
  {"Update start", &_update_start, "UPD_START", false},
  {"Update finish", &_update_finish, "UPD_FINISH", false},
  {"Dev fleet", &_dev_fleet, "BETA_FLT", false},
  {"Alpha fleet", &_alpha_fleet, "ALPHA_FLT", false},
  {"Restore finish", &_restore_finish, "RST_FINISH", false},
  {"Home Assistant enabled", &_ha_en, "HA_EN", false},
  {"EnergieID enabled", &_eid_en, "EID_EN", false}
};

static const intStore configInt[] PROGMEM = {
  {"tempInt", &_tempInt, "TMP_INT", 1}
};

static const uintStore configUInt[] PROGMEM = {
  {"MQTT port", &_mqtt_port, "MQTT_PORT", 1883},
  {"Telegram trigger interval", &_trigger_interval, "TRG_INT", 10},
  {"Telegram trigger type", &_trigger_type, "TRG_TYPE", 0}
};

static const ulongStore configULong[] PROGMEM = {
  {"New firmware version", &_fw_new, "FW_NEW", 0},
  {"Upload throttle", &_upload_throttle, "UPL_THROTTLE", 0}
};

static const stringStore configString[] PROGMEM = {
  {"WiFi SSID", &_wifi_ssid, "WIFI_SSID", ""}, 
  {"MQTT hostname", &_mqtt_host, "MQTT_HOST", "10.42.0.1"},
  {"MQTT ID", &_mqtt_id, "MQTT_ID", ""},
  {"MQTT username", &_mqtt_user, "MQTT_USER", ""}
};

static const stringStore configPass[] PROGMEM = {
  /*Although also Strings, passwords get their own data store as they are handled differently by the configuration module*/
  {"WiFi password", &_wifi_password, "WIFI_PASSWD", ""},
  {"MQTT password", &_mqtt_pass, "MQTT_PASS", ""}
};

static const floatStore configFloat[] PROGMEM = {
  {"tempFloat", &_tempFloat, "TMP_FLT", 1}
};
