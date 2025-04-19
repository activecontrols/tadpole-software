#ifndef PRESSURE_SENSOR_H
#define PRESSURE_SENSOR_H

// See docs/Sensor File.md for info

#include "ADS131M0x.h"

class PressureSensor : ADS131M0x {

private:
  float slope;

public:
  float offset; // public to allow for calibration
  PressureSensor(int demuxAddr, float slope, float offset);
  void begin();
  float getPressure();
  void zero();
};

namespace PT {
void begin();
void zero();

extern PressureSensor lox_tank;
extern PressureSensor lox_venturi_upstream;
extern PressureSensor lox_venturi_throat;

extern PressureSensor ipa_tank;
extern PressureSensor ipa_venturi_upstream;
extern PressureSensor ipa_venturi_throat;

extern PressureSensor chamber;
} // namespace PT

#endif // PRESSURE_SENSOR_H