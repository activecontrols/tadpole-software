#include <Arduino.h>

#include "PressureSensor.h"

PressureSensor::PressureSensor(unsigned int pin) {
  this->pin = pin;
  pinMode(pin, INPUT);
}

void PressureSensor::tare() {
  float p_sum = 0;
  for (int i = 0; i < 10; i++) {
    p_sum += analogRead(pin);
    delay(100);
  }
  tare_offset = p_sum / 10;
}

float PressureSensor::getPressure() {
  float pressure = analogRead(pin) - tare_offset;
  return map(pressure, 0, 1023, 0, 3.3);
}

namespace Pressure {
PressureSensor lox_pressure_in(21);
PressureSensor lox_pressure_out(22);
// PressureSensor ipa_pressure_in(21);
// PressureSensor ipa_pressure_out(22);
} // namespace Pressure
