String dsmrKeys[][ 7 ] = {
/* 
 *  0: string
 *  1: int without unit
 *  2: float without unit
 *  3: int with unit
 *  4: float with unit
 *  5: timestamped Mbus message
 */
{ "1-0:1.8.1", "4", "Total consumption day", "data/devices/utility_meter/energy_consumed_t1", "electricityImport", "cumulative", "energy" },
{ "1-0:1.8.2", "4", "Total consumption night", "data/devices/utility_meter/energy_consumed_t2", "electricityImport", "cumulative", "energy" },
{ "1-0:2.8.1", "4", "Total injection day", "data/devices/utility_meter/energy_injected_t1", "electricityExport", "cumulative", "energy"  }, 
{ "1-0:2.8.2", "4", "Total injection night", "data/devices/utility_meter/energy_injected_t2", "electricityExport", "cumulative", "energy"  },
{ "0-0:96.14.0", "1", "Active tariff period", "data/devices/utility_meter/active_tariff", "", "gauge", "" },  
{ "1-0:1.7.0", "4", "Active power consumption", "data/devices/utility_meter/active_power_consumption", "electricityImport", "gauge", "power" },
{ "1-0:2.7.0", "4", "Active power injection", "data/devices/utility_meter/active_power_injection", "electricityExport", "gauge", "power" },
{ "1-0:32.7.0", "4", "Voltage phase 1", "data/devices/utility_meter/voltage_phase_l1", "", "gauge", "voltage" },
{ "1-0:52.7.0", "4", "Voltage phase 2", "data/devices/utility_meter/voltage_phase_l2", "", "gauge", "voltage" },
{ "1-0:72.7.0", "4", "Voltage phase 3", "data/devices/utility_meter/voltage_phase_l3", "", "gauge", "voltage" },
{ "0-1:24.2.3", "5", "Natural gas consumption", "data/devices/utility_meter/natural_gas_consumption", "naturalGasImport", "cumulative", "" }
//,{ "0-0:96.13.0", "0", "Text message", "data/devices/utility_meter/text_message", "", "gauge", "" }
};
