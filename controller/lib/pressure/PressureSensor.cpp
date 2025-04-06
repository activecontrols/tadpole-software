#include "PressureSensor.h"
#include "SPI_Demux.h"

PressureSensor::PressureSensor(int demuxAddr, float slope, float offset) : ADS131M0x(demuxAddr) {
  this->slope = slope;
  this->offset = offset;
}

void PressureSensor::begin() {
  setInputChannelSelection(0, INPUT_CHANNEL_MUX_AIN0P_AIN0N);
  setInputChannelSelection(1, INPUT_CHANNEL_MUX_AIN0P_AIN0N);
  writeRegister(0x4, 0);
  setChannelOffsetCalibration(0, 0);
  setChannelOffsetCalibration(1, 0);
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
PressureSensor water_tank(SPI_DEVICE_PT_WATER_TANK, 8.18, 9.34);                         // S/N 5920590
PressureSensor water_venturi_upstream(SPI_DEVICE_PT_WATER_VENTURI_UPSTREAM, 8.37, 8.68); // S/N 5920589
PressureSensor water_venturi_throat(SPI_DEVICE_PT_WATER_VENTURI_THROAT, 175.24, 121.69); // S/N 10503469

void begin() {
  water_tank.begin();
  water_venturi_upstream.begin();
  water_venturi_throat.begin();
}
} // namespace PT
