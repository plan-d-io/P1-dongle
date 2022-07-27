static const String dsmrKeys[][ 8 ] PROGMEM = {
/* 
 *  0: string
 *  1: int without unit
 *  2: float without unit
 *  3: int with unit
 *  4: float with unit
 *  5: timestamped Mbus message
 */
{ "1-0:1.8.1", "4", "Total consumption day", "data/devices/utility_meter/total_consumption_day", "GridElectricityImport", "cumulative", "energy" },
{ "1-0:1.8.2", "4", "Total consumption night", "data/devices/utility_meter/total_consumption_night", "GridElectricityImport", "cumulative", "energy" },
{ "1-0:2.8.1", "4", "Total injection day", "data/devices/utility_meter/total_injection_day", "GridElectricityExport", "cumulative", "energy" }, 
{ "1-0:2.8.2", "4", "Total injection night", "data/devices/utility_meter/total_injection_night", "GridElectricityExport", "cumulative", "energy" },
{ "0-0:96.14.0", "1", "Active tariff period", "data/devices/utility_meter/active_tariff_period", "", "gauge", "" },  
{ "1-0:1.7.0", "4", "Active power consumption", "data/devices/utility_meter/active_power_consumption", "GridElectricityPower", "gauge", "power" },
{ "1-0:2.7.0", "4", "Active power injection", "data/devices/utility_meter/active_power_injection", "GridElectricityPower", "gauge", "power" },
{ "1-0:32.7.0", "4", "Voltage phase 1", "data/devices/utility_meter/voltage_phase_1", "GridElectricityVoltage", "gauge", "voltage"  },
{ "1-0:52.7.0", "4", "Voltage phase 2", "data/devices/utility_meter/voltage_phase_2", "GridElectricityVoltage", "gauge", "voltage" },
{ "1-0:72.7.0", "4", "Voltage phase 3", "data/devices/utility_meter/voltage_phase_3", "GridElectricityVoltage", "gauge", "voltage" },
{ "1-0:31.7.0", "4", "Current phase 1", "data/devices/utility_meter/current_phase_1", "GridElectricityCurrent", "gauge", "current" },
{ "1-0:51.7.0", "4", "Current phase 2", "data/devices/utility_meter/current_phase_2", "GridElectricityCurrent", "gauge", "current" },
{ "1-0:71.7.0", "4", "Current phase 3", "data/devices/utility_meter/current_phase_3", "GridElectricityCurrent", "gauge", "current" },
{ "0-1:24.2.3", "5", "Natural gas consumption", "data/devices/utility_meter/natural_gas_consumption", "naturalGasImport", "cumulative", "gas" },
{ "0-0:96.13.0", "0", "Text message", "data/devices/utility_meter/text_message", "", "gauge", "" }
};

void configMeter(){
  meterConfig[0] = "1";
  meterConfig[1] = "1";
  meterConfig[2] = "1";
  meterConfig[3] = "1";
  meterConfig[4] = dmActiveTariff;
  meterConfig[5] = "1";
  meterConfig[6] = "1";
  meterConfig[7] = dmVoltagel1;
  meterConfig[8] = dmVoltagel2;
  meterConfig[9] = dmVoltagel3;
  meterConfig[10] = dmCurrentl1;
  meterConfig[11] = dmCurrentl2;
  meterConfig[12] = dmCurrentl3;
  meterConfig[13] = dmGas;
  meterConfig[14] = dmText;
}
