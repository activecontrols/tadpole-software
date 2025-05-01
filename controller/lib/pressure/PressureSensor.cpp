#include "PressureSensor.h"
#include "SPI_Demux.h"
#include "Router.h"

// #define PRINT_PT_MV // enable to print adc mv measurement (for calibration)

PressureSensor::PressureSensor(int demuxAddr, float slope) : ADS131M0x(demuxAddr) {
  this->slope = slope;
  this->offset = 0;
  this->last_good_value = 0;
}

void PressureSensor::begin() {
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
    return last_good_value;
  }

  // if (!(out.status & 0b1)) {
  //   Serial.print(out.status, HEX);
  //   Serial.print(" rdy err ");
  // }
  float voltage = out.ch1;
  voltage *= 2.4 / 1;
  voltage /= -pow(2, 24);

#ifdef PRINT_PT_MV
  Serial.print(voltage * 1000);
  Serial.print(" mV ");
#endif
  float rval = map(voltage, 0, 1, 0, 10) * slope + offset;
  last_good_value = rval;
  return rval;
}

void PressureSensor::zero(float target) {
  offset = 0;
  float sum = 0;
  int samples = 100;
  for (int i = 0; i < samples; i++) {
    sum += getPressure();
    delay(10);
  }
  offset = target - sum / samples;
}

namespace PT {
bool zeroed_since_boot;
PressureSensor lox_valve_upstream(SPI_DEVICE_PT_LOX_VALVE_UPSTREAM, 155.98);
PressureSensor lox_valve_downstream(SPI_DEVICE_PT_LOX_VALVE_DOWNSTREAM, 103.37); // not used during throttle
PressureSensor lox_venturi_differential(SPI_DEVICE_PT_LOX_VENTURI_DIFFERENTIAL, 4.97);

PressureSensor ipa_valve_upstream(SPI_DEVICE_PT_IPA_VALVE_UPSTREAM, 158.70);
PressureSensor ipa_valve_downstream(SPI_DEVICE_PT_IPA_VALVE_DOWNSTREAM, 157.92); // not used during throttle
PressureSensor ipa_venturi_differential(SPI_DEVICE_PT_IPA_VENTURI_DIFFERENTIAL, 31.00);

PressureSensor chamber(SPI_DEVICE_PT_CHAMBER, 51.07);

void begin() {
  lox_valve_upstream.begin();
  lox_valve_downstream.begin();
  lox_venturi_differential.begin();

  ipa_valve_upstream.begin();
  ipa_valve_downstream.begin();
  ipa_venturi_differential.begin();

  chamber.begin();

  Router::add({zero, "zero_pt_to_atm"});
}

void zero() {
  Router::info_no_newline("Zeroing ...");
  lox_valve_upstream.zero(14.7);
  lox_valve_downstream.zero(14.7);
  lox_venturi_differential.zero(0);

  ipa_valve_upstream.zero(14.7);
  ipa_valve_downstream.zero(14.7);
  ipa_venturi_differential.zero(0);

  chamber.zero(14.7);
  zeroed_since_boot = true;
  Router::info(" finished!");

  Router::info_no_newline("LOX Valve Upstream Offset (expected -38): ");
  Router::info(lox_valve_upstream.offset);

  Router::info_no_newline("LOX Valve Downstream Offset (expected -30): ");
  Router::info(lox_valve_downstream.offset);

  Router::info_no_newline("IPA Valve Upstream Offset (expected -70): ");
  Router::info(ipa_valve_upstream.offset);

  Router::info_no_newline("IPA Valve Downstream Offset (expected -78): ");
  Router::info(ipa_valve_downstream.offset);

  Router::info_no_newline("Chamber Offset (expected -10): ");
  Router::info(chamber.offset);

  Router::info_no_newline("LOX Diffy Offset (-1): ");
  Router::info(lox_venturi_differential.offset);

  Router::info_no_newline("IPA Diffy Offset (-12): ");
  Router::info(ipa_venturi_differential.offset);
}
} // namespace PT
