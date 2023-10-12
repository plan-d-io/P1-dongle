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
{ "1-0:1.7.0", "GridElectricityPower", "gauge" },
{ "1-0:2.7.0", "GridElectricityPower", "gauge" },
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

/*Re.alto*/
elapsedMillis lastRealtoUpload;
unsigned int realtoUploadTries;
/* Configure the keys to be included in the realto payload */
static const String realtoKeys[] PROGMEM = {
{ "1-0:1.8.1", },
{ "1-0:1.8.2", },
{ "1-0:2.8.1", }, 
{ "1-0:2.8.2", },
{ "1-0:1.7.0", },
{ "1-0:2.7.0", },
{ "1-0:1.4.0", },
{ "1-0:1.6.0", },
{ "1-0:32.7.0", },
{ "1-0:52.7.0", },
{ "1-0:72.7.0", },
{ "1-0:31.7.0", },
{ "1-0:51.7.0", },
{ "1-0:71.7.0", },
{ "0-1:24.2.3", }
};
   
