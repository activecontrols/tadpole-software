#include "Thermocouples.h"
#include "spi_demux.hpp"

Thermocouple::Thermocouple(int demuxAddr) : Adafruit_MAX31856(demuxAddr) {
}

void Thermocouple::begin() {
  // assert on any fault
  this->writeRegister8(MAX31856_MASK_REG, 0x0);

  // enable open circuit fault detection
  writeRegister8(MAX31856_CR0_REG, MAX31856_CR0_OCFAULT0);

  // set cold junction temperature offset to zero
  writeRegister8(MAX31856_CJTO_REG, 0x0);

  // set Type K by default
  setThermocoupleType(MAX31856_TCTYPE_K);

  // set One-Shot conversion mode
  setConversionMode(MAX31856_ONESHOT);
}

float Thermocouple::getTemperature() {
  return readThermocoupleTemperature();
}

namespace TC {
Thermocouple example_tc(SPI_DEVICE_TC_0);

void begin() {
  example_tc.begin();
}
} // namespace TC