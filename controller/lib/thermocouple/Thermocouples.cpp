#include "Thermocouples.h"
#include "SPI_Demux.h"

Thermocouple::Thermocouple(int demuxAddr) : Adafruit_MAX31856(demuxAddr) {
}

void Thermocouple::begin(max31856_thermocoupletype_t tc_type) {
  // assert on any fault
  writeRegister8(MAX31856_MASK_REG, 0x0);

  // enable open circuit fault detection
  writeRegister8(MAX31856_CR0_REG, MAX31856_CR0_OCFAULT0);

  // set cold junction temperature offset to zero
  writeRegister8(MAX31856_CJTO_REG, 0x0);

  // set Type K by default
  setThermocoupleType(tc_type);

  // set One-Shot conversion mode
  setConversionMode(MAX31856_CONTINUOUS);
}

float Thermocouple::getTemperature_F() {
  return readThermocoupleTemperature() * 9.0 / 5.0 + 32;
}

float Thermocouple::getTemperature_Kelvin() {
  return readThermocoupleTemperature() + 273.15;
}

namespace TC {
Thermocouple lox_venturi_temperature(SPI_DEVICE_TC_LOX_VENTURI);
Thermocouple lox_valve_temperature(SPI_DEVICE_TC_LOX_VALVE);

void begin() {
  lox_venturi_temperature.begin(MAX31856_TCTYPE_E);
  lox_valve_temperature.begin(MAX31856_TCTYPE_E);
}
} // namespace TC