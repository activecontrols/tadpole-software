#ifndef PRESSURE_SENSOR_H
#define PRESSURE_SENSOR_H

// See docs/Sensor File.md for info

#include "ADS131M0x.h"

class PressureSensor : ADS131M0x {

private:
  float slope;
  float offset;

public:
  PressureSensor(int demuxAddr, float slope, float offset);
  void begin();
  float getPressure();
};

namespace PT {
void begin();

extern PressureSensor water_tank;
extern PressureSensor water_venturi_upstream;
extern PressureSensor water_venturi_throat;
} // namespace PT

#endif // PRESSURE_SENSOR_H