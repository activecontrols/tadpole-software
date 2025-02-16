#include "Thermocouples.h"
#include "spi_demux.hpp"

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

float Thermocouple::getTemperature() {
  return readThermocoupleTemperature() * 9.0 / 5.0 + 32;
}

namespace TC {
Thermocouple example_tc(SPI_DEVICE_TC_0);

void begin() {
  example_tc.begin(MAX31856_TCTYPE_K);
}
} // namespace TC