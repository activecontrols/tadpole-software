#include "PressureSensor.h"
#include "SPI_Demux.h"

PressureSensor::PressureSensor(int demuxAddr, float slope, float offset) : ADS131M0x(demuxAddr) {
  this->slope = slope;
  this->offset = offset;
}

void PressureSensor::begin() {
  // setInputChannelSelection(0, INPUT_CHANNEL_MUX_AIN0P_AIN0N);
  // setInputChannelSelection(1, INPUT_CHANNEL_MUX_AIN0P_AIN0N);
  // setChannelPGA(0, 0);
  // setChannelPGA(1, 0);
  // setChannelOffsetCalibration(0, 0);
  // setChannelOffsetCalibration(1, 0);
  setChannelGainCalibration(0, 0); // sets both gains to 0 - if the reset works, they will be reset to 1
  setChannelGainCalibration(1, 0); // sets both gains to 0 - if the reset works, they will be reset to 1
  resetDevice();
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
PressureSensor pressure_0(SPI_DEVICE_PT_0, 100, 0);
PressureSensor pressure_2(SPI_DEVICE_PT_2, 100, 0);
PressureSensor pressure_4(SPI_DEVICE_PT_4, 100, 0);

PressureSensor pressure_6(SPI_DEVICE_PT_6, 100, 0);
PressureSensor pressure_8(SPI_DEVICE_PT_8, 100, 0);
PressureSensor pressure_10(SPI_DEVICE_PT_10, 100, 0);

PressureSensor pressure_12(SPI_DEVICE_PT_12, 100, 0);

void begin() {
  pressure_0.begin();
  pressure_2.begin();
  pressure_4.begin();

  pressure_6.begin();
  pressure_8.begin();
  pressure_10.begin();

  pressure_12.begin();
}
} // namespace PT
