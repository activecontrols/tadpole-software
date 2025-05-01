#ifndef PRESSURE_SENSOR_H
#define PRESSURE_SENSOR_H

// See docs/Sensor File.md for info

#include "ADS131M0x.h"

class PressureSensor : ADS131M0x {

private:
  float slope;
  float last_good_value;

public:
  float offset; // public to allow for calibration
  PressureSensor(int demuxAddr, float slope);
  void begin();
  float getPressure();
  void zero(float target);
};

namespace PT {
void begin();
void zero();
extern bool zeroed_since_boot;

extern PressureSensor lox_valve_upstream;
extern PressureSensor lox_valve_downstream;
extern PressureSensor lox_venturi_differential;

extern PressureSensor ipa_valve_upstream;
extern PressureSensor ipa_valve_downstream;
extern PressureSensor ipa_venturi_differential;

extern PressureSensor chamber;
} // namespace PT

#endif // PRESSURE_SENSOR_H