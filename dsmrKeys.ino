static const String dsmrKeys[][ 8 ] PROGMEM = {
/* 
 *  0: string
 *  1: int without unit
 *  2: float without unit
 *  3: int with unit
 *  4: float with unit
 *  5: timestamped Mbus message
 */
{ "1-0:1.8.1", "4", "Total consumption T1", "data/devices/utility_meter/total_consumption_t1", "", "cumulative", "energy" },
{ "1-0:1.8.2", "4", "Total consumption T2", "data/devices/utility_meter/total_consumption_t2", "", "cumulative", "energy" },
{ "1-0:2.8.1", "4", "Total injection T1", "data/devices/utility_meter/total_injection_t1", "", "cumulative", "energy" }, 
{ "1-0:2.8.2", "4", "Total injection T2", "data/devices/utility_meter/total_injection_t2", "", "cumulative", "energy" },
{ "0-0:96.14.0", "1", "Active tariff period", "data/devices/utility_meter/active_tariff_period", "", "gauge", "" },  
{ "1-0:1.7.0", "4", "Active power consumption", "data/devices/utility_meter/active_power_consumption", "GridElectricityPower", "gauge", "power" },
{ "1-0:2.7.0", "4", "Active power injection", "data/devices/utility_meter/active_power_injection", "GridElectricityPower", "gauge", "power" },
{ "1-0:1.4.0", "4", "Current average demand", "data/devices/utility_meter/current_average_demand", "", "gauge", "power" },
{ "1-0:1.6.0", "5", "Maximum demand current month", "data/devices/utility_meter/maximum_demand_current_month", "", "gauge", "power" },
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
  meterConfig[0] = dmTotCont1;
  meterConfig[1] = dmTotCont2;
  meterConfig[2] = dmTotInt1;
  meterConfig[3] = dmTotInt2;
  meterConfig[4] = dmActiveTariff;
  meterConfig[5] = dmPowCon;
  meterConfig[6] = dmPowIn;
  meterConfig[7] = dmAvDem;
  meterConfig[8] = dmMaxDemM;
  meterConfig[9] = dmVoltagel1;
  meterConfig[10] = dmVoltagel2;
  meterConfig[11] = dmVoltagel3;
  meterConfig[12] = dmCurrentl1;
  meterConfig[13] = dmCurrentl2;
  meterConfig[14] = dmCurrentl3;
  meterConfig[15] = dmGas;
  meterConfig[16] = dmText;
}
