#include <Arduino.h>

#include "PressureObj.h"

PressureObj::PressureObj(unsigned int pin, float pressureMin, float pressureMax, float voltageMin, float voltageMax, float filter_cutoff_freq, float filter_sample_freq) {
    this->pin = pin;
    this->pressureMin = pressureMin;
    this->pressureMax = pressureMax;
    this->voltageMin = voltageMin;
    this->voltageMax = voltageMax;
    this->filter = BiquadLPF(filter_cutoff_freq, filter_sample_freq);
}

float PressureObj::getPressure() {
    float pressure = analogRead(pin);
    return filter.biquadLPFApply(map(pressure, voltageMin, voltageMax, pressureMin, pressureMax));
}
