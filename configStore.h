/*In this file you can add and configure global variables stored in non-volatile (NVS) data storage*/

/*The following bespoke structures are used to construct NVS data storages for each data type*/
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
bool _wifi_STA;
String _wifi_ssid, _wifi_password;
/*User*/
String _user_email;
/*MQTT*/
bool _mqtt_en, _mqtt_tls, _mqtt_auth;
unsigned int _mqtt_port;
String _mqtt_host, _mqtt_id, _mqtt_user, _mqtt_pass, _mqtt_prefix;
/*Update*/
bool _update_auto, _update_autoCheck, _update_start, _update_finish, _dev_fleet, _alpha_fleet, _v2_fleet, _restore_finish;
unsigned long _fw_new;
String _rel_chan;
/*Debug*/
bool _reinit_spiffs;
unsigned int _bootcount;
String _last_reset;
/*DSMR processing*/
bool _push_full_telegram;
unsigned long _key_pushlist, _upload_throttle;
unsigned int _mbus_pushlist, _trigger_interval, _trigger_type;
int _payload_format;
/*External services*/
bool _ha_en, _eid_en, _realto_en;
String _ha_device;
unsigned long _realtoThrottle;
/*Placeholder vars*/
float _tempFloat;
String _tempString;

/*The configuration data stores for every data type.
 * Format: { "User-readable name", global variable name (reference), "NVS key name", default value }
 */
static const boolStore configBool[] PROGMEM = {
  {"WiFi Station mode", &_wifi_STA, "WIFI_STA", false}, 
  {"MQTT enabled", &_mqtt_en, "MQTT_EN", false},
  {"MQTT secure", &_mqtt_tls, "MQTT_TLS", false},
  {"MQTT auth", &_mqtt_auth, "MQTT_AUTH", false},
  {"Full DSMR telegram", &_push_full_telegram, "PUSH_FULL", false},
  {"Update auto", &_update_auto, "UPD_AUTO", true},
  {"Update autocheck", &_update_autoCheck, "UPD_AUTOCHK", true},
  {"Update start", &_update_start, "UPD_START", false},
  {"Update finish", &_update_finish, "UPD_FINISH", false},
  {"Reinitialise SPIFFS", &_reinit_spiffs, "RINT_SPIFFS", false},
  {"Dev fleet", &_dev_fleet, "BETA_FLT", false},
  {"Alpha fleet", &_alpha_fleet, "ALPHA_FLT", false},
  {"V2 fleet", &_v2_fleet, "V2_FLT", false},
  {"Restore finish", &_restore_finish, "RST_FINISH", false},
  {"Home Assistant enabled", &_ha_en, "HA_EN", false},
  {"Re.alto enabled", &_realto_en, "RLT_EN", false},
  {"EnergieID enabled", &_eid_en, "EID_EN", false}
};

static const intStore configInt[] PROGMEM = {
  {"Data payload format", &_payload_format, "FRMT_PYLD", 2} //0 = value only, 1 = minimal json, 2 = standard json, 3 = COFY format
};

static const uintStore configUInt[] PROGMEM = {
  {"MBus keys", &_mbus_pushlist, "PUSH_MBUS", 136},
  {"MQTT port", &_mqtt_port, "MQTT_PORT", 1883},
  {"Telegram trigger interval", &_trigger_interval, "TRG_INT", 10},
  {"Telegram trigger type", &_trigger_type, "TRG_TYPE", 0},
  {"Bootcount", &_bootcount, "reboots", 0}
};

static const ulongStore configULong[] PROGMEM = {
  {"DSMR keys", &_key_pushlist, "PUSH_DSMR", 65534},
  //{"New firmware version", &_fw_new, "FW_NEW", 0},
  {"Upload throttle", &_upload_throttle, "UPL_THROTTLE", 0},
  {"Re.alto throttle", &_realtoThrottle, "RLT_THROTTLE", 60}
};

static const stringStore configString[] PROGMEM = {
  {"WiFi network", &_wifi_ssid, "WIFI_SSID", ""},
  {"MQTT hostname", &_mqtt_host, "MQTT_HOST", "10.42.0.1"},
  {"MQTT ID", &_mqtt_id, "MQTT_ID", ""},
  {"MQTT username", &_mqtt_user, "MQTT_USER", ""},
  {"MQTT topic prefix", &_mqtt_prefix, "MQTT_PFIX", "data/devices/beta_meter/"},
  {"HA device name", &_ha_device, "HA_DEVICE", "Beta meter"},
  {"Last reset reason (firmware)", &_last_reset, "LAST_RESET", ""},
  {"Release channel", &_rel_chan, "REL_CHAN", "main"},
  {"Temp string", &_tempString, "TMP_STR", ""},
  {"User email", &_user_email, "EMAIL", ""}
};

static const stringStore configPass[] PROGMEM = {
  /*Although also Strings, passwords get their own data store as they are never returned as plaintext (contrary to Strings)
    This store can also be used for GDPR sensitive information, e.g. user e-mails.*/
  {"WiFi password", &_wifi_password, "WIFI_PASSWD", ""},
  {"MQTT password", &_mqtt_pass, "MQTT_PASS", ""}
};

static const floatStore configFloat[] PROGMEM = {
  {"tempFloat", &_tempFloat, "TMP_FLT", 1}
};
