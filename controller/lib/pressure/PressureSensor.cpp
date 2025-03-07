#include <Arduino.h>

#include "PressureSensor.h"
#include "spi_demux.hpp"

PressureSensor::PressureSensor(int demuxAddr, float slope, float offset) : ADS131M0x(demuxAddr) {
  this->slope = slope;
  this->offset = offset;
}

void PressureSensor::begin() {
  setInputChannelSelection(0, INPUT_CHANNEL_MUX_AIN0P_AIN0N);
  setInputChannelSelection(1, INPUT_CHANNEL_MUX_AIN0P_AIN0N);
}

// returns the absolute pressure
// first the incoming PT data is mapped to 0 to 10v
// this value is multiplied by the slope (should be approximately PT range / 10) and a small linear offset is added
// to find these values, measure a few points of **ABSOLUTE** pressure and the corresponding voltages
// plot as voltage on x and pressure on y, and find the line of best fit
float PressureSensor::getPressure() {
  adcOutput out = this->readADC();
  if (!(out.status & 0b1)) {
    Serial.print(out.status, HEX);
    Serial.print(" err ");
  }
  float voltage = out.ch1;
  voltage *= 2.4 / 1;
  voltage /= pow(2, 24);
  return map(voltage, 0, 1, 0, 10) * slope + offset;
}

namespace Pressure {
PressureSensor lox_pressure_in(SPI_DEVICE_PT_0, 9.38, 2.3);
PressureSensor lox_pressure_out(SPI_DEVICE_PT_1, 9.38, 2.3);
// PressureSensor ipa_pressure_in(21);
// PressureSensor ipa_pressure_out(22);

void begin() {
  lox_pressure_in.begin();
  lox_pressure_out.begin();
}
} // namespace Pressure
