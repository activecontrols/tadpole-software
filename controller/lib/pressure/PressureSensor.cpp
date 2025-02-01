#include <Arduino.h>

#include "PressureSensor.h"
#include "teensy_pins.hpp"

PressureSensor::PressureSensor(unsigned int pin, float slope, float offset) {
  this->pin = pin;
  this->slope = slope;
  this->offset = offset;
  pinMode(pin, INPUT);
}

// returns the absolute pressure
// first the incoming PT data is mapped to 0 to 10v
// this value is multiplied by the slope (should be approximately PT range / 10) and a small linear offset is added
// to find these values, measure a few points of **ABSOLUTE** pressure and the corresponding voltages
// plot as voltage on x and pressure on y, and find the line of best fit
float PressureSensor::getPressure() {
  float pressure = analogRead(pin);
  return map(pressure, 0, 1023, 0, 10) * slope + offset;
}

namespace Pressure {
PressureSensor lox_pressure_in(LOX_PRESSURE_UPSTREAM_PIN, 9.38, 2.3);
PressureSensor lox_pressure_out(LOX_PRESSURE_DOWNSTREAM_PIN, 9.38, 2.3);
// PressureSensor ipa_pressure_in(21);
// PressureSensor ipa_pressure_out(22);
} // namespace Pressure
