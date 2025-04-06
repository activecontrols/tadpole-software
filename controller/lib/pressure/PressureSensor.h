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

extern PressureSensor pressure_0;
extern PressureSensor pressure_2;
extern PressureSensor pressure_4;

extern PressureSensor pressure_6;
extern PressureSensor pressure_8;
extern PressureSensor pressure_10;

extern PressureSensor pressure_12;
} // namespace PT

#endif // PRESSURE_SENSOR_H