#include <Arduino.h>

#include "PressureObj.h"

PressureObj::PressureObj(unsigned int pin, float pressureMin, float pressureMax, float voltageMin, float voltageMax) {
    this->pin = pin;
    this->pressureMin = pressureMin;
    this->pressureMax = pressureMax;
    this->voltageMin = voltageMin;
    this->voltageMax = voltageMax;
}

float PressureObj::getPressure() {
    float pressure = analogRead(pin);
    return map(pressure, voltageMin, voltageMax, pressureMin, pressureMax);
}
