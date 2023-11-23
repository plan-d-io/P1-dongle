/*Add third-party, external integrations configuration options here*/

/* Add additional COFY field to the MQTT payload */
static const String cofyKeys[][ 3 ] PROGMEM = {
{ "A-0:0.0.1", "GridElectricityImport", "cumulative"},
{ "A-0:0.0.2", "GridElectricityExport", "cumulative"},
{ "A-0:0.0.3", "GridElectricityPower", "gauge"},
{ "1-0:1.8.1", "", "cumulative" },
{ "1-0:1.8.2", "", "cumulative" },
{ "1-0:2.8.1", "", "cumulative" }, 
{ "1-0:2.8.2", "", "cumulative" },
{ "0-0:96.14.0", "", "gauge" },  
{ "1-0:1.7.0", "", "gauge" },
{ "1-0:2.7.0", "", "gauge" },
{ "1-0:1.4.0", "", "gauge" },
{ "1-0:1.6.0", "", "gauge" },
{ "1-0:32.7.0", "GridElectricityVoltage", "gauge" },
{ "1-0:52.7.0", "GridElectricityVoltage", "gauge" },
{ "1-0:72.7.0", "GridElectricityVoltage", "gauge" },
{ "1-0:31.7.0", "GridElectricityCurrent", "gauge" },
{ "1-0:51.7.0", "GridElectricityCurrent", "gauge" },
{ "1-0:71.7.0", "GridElectricityCurrent", "gauge" },
{ "0-1:24.2.3", "naturalGasImport", "cumulative" },
{ "0-0:96.13.0", "", "gauge" }
};

/*EnergieID (EID)*/
bool EIDuploadEn = false;
elapsedMillis lastEIDupload, lastEIDcheck;
unsigned long EIDcheckInterval = 150000;
unsigned long EIDuploadInterval, EIDexpire;
String EIDclaimUrl, EIDwebhookUrl, EIDauthorization, EIDxtwinid;
/*Add keys to be sent to EnergieID here. Values of gas and water (if present) and peak power (capacity) are automatically included*/
static const String eidDsmrKeys[][2] PROGMEM = {
{ "t1", "1-0:1.8.1", },
{ "t2", "1-0:1.8.2", },
{ "t1-i", "1-0:2.8.1", }, 
{ "t2-i", "1-0:2.8.2", },
{ "pwr", "1-0:1.7.0", },
{ "pwr-i", "1-0:2.7.0", },
{ "pp", "1-0:1.4.0", },
{ "volt1", "1-0:32.7.0" },
{ "cur1", "1-0:31.7.0"}
};
