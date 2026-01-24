#ifndef ENCODER_H
#define ENCODER_H

// See docs/Sensor File.md for info

#include "AMT242AV.h"

namespace Encoder {
void print_pos();
void begin();
void zero();

extern AMT242AV encoder;
} // namespace Encoder

#endif // PRESSURE_SENSOR_H