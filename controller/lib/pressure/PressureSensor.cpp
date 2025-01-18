#include <Arduino.h>

#include "PressureSensor.h"

PressureSensor::PressureSensor(unsigned int pin) {
  this->pin = pin;
  pinMode(pin, INPUT);
}

float PressureSensor::getPressure() {
  float pressure = analogRead(pin);
  return map(pressure, 0, 1023, 0, 3.3);
}

namespace Pressure {
PressureSensor lox_pressure_in(21);
PressureSensor lox_pressure_out(22);
// PressureSensor ipa_pressure_in(21);
// PressureSensor ipa_pressure_out(22);
} // namespace Pressure
