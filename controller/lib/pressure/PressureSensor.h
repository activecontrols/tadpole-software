#ifndef PRESSURE_SENSOR_H
#define PRESSURE_SENSOR_H

/*
 * PressureSensor.h
 *
 *  Created on: 2024-6-19 by Vincent Palmerio
 *  Description: This file contains the declaration of the PressureSensor class,
 *  in which each object is used to read from a specific pressure sensor.
 */

class PressureSensor {

private:
  unsigned int pin;
  float tare_offset = 0;

public:
  PressureSensor(unsigned int pin);
  void tare();
  float getPressure();
};

namespace Pressure {
extern PressureSensor lox_pressure_in;
extern PressureSensor lox_pressure_out;
// extern PressureSensor ipa_pressure_in;
// extern PressureSensor ipa_pressure_out;
} // namespace Pressure

#endif // PRESSURE_SENSOR_H