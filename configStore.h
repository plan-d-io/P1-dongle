/*In this file you can add and configure global variables stored in non-volatile (NVS) data storage*/

/*The following bespoke structures are used to construct NVS data storages for each data type*/
struct boolStore{
   String varName;
   bool* var;
   String configName;
   bool defaultValue;
   bool includeInConfig;
};

struct intStore{
   String varName;
   int* var;
   String configName;
   int defaultValue;
   bool includeInConfig;
};

struct uintStore{
   String varName;
   unsigned int* var;
   String configName;
   unsigned int defaultValue;
   bool includeInConfig;
};

struct ulongStore{
   String varName;
   unsigned long* var;
   String configName;
   unsigned long defaultValue;
   bool includeInConfig;
};

struct ipStore{
   String varName;
   uint32_t* var;
   String configName;
   unsigned long defaultValue;
   bool includeInConfig;
};

struct stringStore{
   String varName;
   String* var;
   String configName;
   String defaultValue;
   bool includeInConfig;
};

struct floatStore{
   String varName;
   float* var;
   String configName;
   float defaultValue;
   bool includeInConfig;
};

/*Declaration of global variables retrieved from NVS config storage, denoted by a leading _*/
/*Dongle*/
String _uuid;
/*Wifi*/
bool _wifi_STA, _fip_en;
String _wifi_ssid, _wifi_password;
uint32_t _fipaddr, _fdefgtw, _fsubn, _fdns1, _fdns2;
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
unsigned int _rebootSecure, _bootcount;
String _last_reset;
/*DSMR processing*/
bool _push_full_telegram;
unsigned long _key_pushlist, _upload_throttle;
unsigned int _mbus_pushlist, _trigger_interval, _trigger_type;
int _payload_format;
/*External services*/
bool _ha_en, _eid_en, _realto_en;
String _ha_device;
String _eid_provkey, _eid_provsec, _eidclaim;
unsigned long _realtoThrottle;
/*Placeholder vars*/
float _tempFloat;
String _tempString;

/*The configuration data stores for every data type.
 * Format: { "User-readable name", global variable name (reference), "NVS key name", default value, include in config API }
 */
static const boolStore configBool[] PROGMEM = {
  {"WiFi Station mode", &_wifi_STA, "WIFI_STA", false, false},
  {"Use fixed IP", &_fip_en, "FIP_EN", false, true},
  {"MQTT enabled", &_mqtt_en, "MQTT_EN", false, true},
  {"MQTT secure", &_mqtt_tls, "MQTT_TLS", false, true},
  {"MQTT auth", &_mqtt_auth, "MQTT_AUTH", false, true},
  {"Full DSMR telegram", &_push_full_telegram, "PUSH_FULL", false, true},
  {"Update auto", &_update_auto, "UPD_AUTO", true, true},
  {"Update autocheck", &_update_autoCheck, "UPD_AUTOCHK", true, true},
  {"Update start", &_update_start, "UPD_START", false, false},
  {"Update finish", &_update_finish, "UPD_FINISH", false, false},
  {"Reinitialise SPIFFS", &_reinit_spiffs, "RINT_SPIFFS", false, true},
  {"Dev fleet", &_dev_fleet, "BETA_FLT", false, true},
  {"Alpha fleet", &_alpha_fleet, "ALPHA_FLT", false, true},
  {"V2 fleet", &_v2_fleet, "V2_FLT", false, true},
  {"Restore finish", &_restore_finish, "RST_FINISH", false, false},
  {"Home Assistant enabled", &_ha_en, "HA_EN", false, true},
  {"EnergieID enabled", &_eid_en, "EID_EN", false, true}
};

static const intStore configInt[] PROGMEM = {
  {"Data payload format", &_payload_format, "FRMT_PYLD", 3, true} //0 = value only, 1 = minimal json, 2 = standard json, 3 = COFY format
};

static const uintStore configUInt[] PROGMEM = {
  {"MBus keys", &_mbus_pushlist, "PUSH_MBUS", 136, false},
  {"MQTT port", &_mqtt_port, "MQTT_PORT", 1883, true},
  {"Telegram trigger interval", &_trigger_interval, "TRG_INT", 10, false},
  {"Telegram trigger type", &_trigger_type, "TRG_TYPE", 0, false},
  {"Reboots secureClient", &_rebootSecure, "RBT_SEC", 0, false},
  {"Bootcount", &_bootcount, "reboots", 0, false}
};

static const ulongStore configULong[] PROGMEM = {
  {"DSMR keys", &_key_pushlist, "PUSH_DSMR", 1073741823, true},
  {"Upload throttle", &_upload_throttle, "UPL_THROTTLE", 10, true}
};

static const stringStore configString[] PROGMEM = {
  {"UUID", &_uuid, "UUID", "", true},
  {"EID claim code", &_eidclaim, "EIDCLAIM", "", true},
  {"WiFi network", &_wifi_ssid, "WIFI_SSID", "", true},
  {"MQTT hostname", &_mqtt_host, "MQTT_HOST", "10.42.0.1", true},
  {"MQTT ID", &_mqtt_id, "MQTT_ID", "", true},
  {"MQTT username", &_mqtt_user, "MQTT_USER", "", true},
  {"MQTT topic prefix", &_mqtt_prefix, "MQTT_PFIX", "data/devices/utility_meter/", true},
  {"HA device name", &_ha_device, "HA_DEVICE", "Utility meter", true},
  {"Last reset reason (firmware)", &_last_reset, "LAST_RESET", "", false},
  {"Release channel", &_rel_chan, "REL_CHAN", "main", true},
  {"User email", &_user_email, "EMAIL", "", true}
};

static const stringStore configPass[] PROGMEM = {
  /*Although also Strings, passwords get their own data store as they are never returned as plaintext (contrary to Strings)
    This store can also be used for GDPR sensitive information, e.g. user e-mails.*/
  {"WiFi password", &_wifi_password, "WIFI_PASSWD", "", true},
  {"MQTT password", &_mqtt_pass, "MQTT_PASS", "", true}
};

static const stringStore configSecret[] PROGMEM = {
  {"EID Provisioning key", &_eid_provkey, "EID_PROVKEY", "", true},
  {"EID Provisioning secret", &_eid_provsec, "EID_PROVSEC", "", true}
};

static const ipStore configIP[] PROGMEM = {
  {"IP address", &_fipaddr, "FIPADDR", 0, true},
  {"Default gateway", &_fdefgtw, "FDEFGTW", 0, true},
  {"Subnet mask", &_fsubn, "FSUBN", 0, true},
  {"Primary DNS", &_fdns1, "FDNS1", 0, true},
  {"Secondary DNS", &_fdns2, "FDNS2", 0, true}
};

static const floatStore configFloat[] PROGMEM = {
  //{"tempFloat", &_tempFloat, "TMP_FLT", 1}
};
