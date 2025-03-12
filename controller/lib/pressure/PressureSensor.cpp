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
  if (!out.crc_ok) {
    Serial.print(" crc err ");
    return -1;
  }

  if (!(out.status & 0b1)) {
    Serial.print(out.status, HEX);
    Serial.print(" rdy err ");
  }
  float voltage = out.ch1;
  voltage *= 2.4 / 1;
  voltage /= pow(2, 24);
  return map(voltage, 0, 1, 0, 10) * slope + offset;
}

namespace PT {
PressureSensor lox_tank(SPI_DEVICE_PT_LOX_TANK, 9.38, 2.3);
PressureSensor lox_venturi_upstream(SPI_DEVICE_PT_LOX_VENTURI_UPSTREAM, 9.38, 2.3);
PressureSensor lox_venturi_throat(SPI_DEVICE_PT_LOX_VENTURI_THROAT, 9.38, 2.3);

PressureSensor ipa_tank(SPI_DEVICE_PT_IPA_TANK, 9.38, 2.3);
PressureSensor ipa_venturi_upstream(SPI_DEVICE_PT_IPA_VENTURI_UPSTREAM, 9.38, 2.3);
PressureSensor ipa_venturi_throat(SPI_DEVICE_PT_IPA_VENTURI_THROAT, 9.38, 2.3);

PressureSensor chamber(SPI_DEVICE_PT_CHAMBER, 9.38, 2.3);

void begin() {
  lox_tank.begin();
  lox_venturi_upstream.begin();
  lox_venturi_throat.begin();

  ipa_tank.begin();
  ipa_venturi_upstream.begin();
  ipa_venturi_throat.begin();

  chamber.begin();
}
} // namespace PT
