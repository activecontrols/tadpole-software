#include <Arduino.h>

#include "PressureObj.h"

PressureObj::PressureObj(unsigned int pin, float pressureMin, float pressureMax, float voltageMin, float voltageMax, float filter_cutoff_freq, float sample_freq) {
    this->pin = pin;
    this->pressureMin = pressureMin;
    this->pressureMax = pressureMax;
    this->voltageMin = voltageMin;
    this->voltageMax = voltageMax;
    this->sample_time = 1000.0/sample_freq;
    this->filter = BiquadLPF(filter_cutoff_freq, sample_freq);
}

void PressureObj::checkPressure(void *castedArgs){
    PressureObj* obj = static_cast<PressureObj*>(castedArgs);

    while(true){
        this->pressure = obj->getPressure();
    }
}

float PressureObj::getPressure() volatile {
    float pressure = analogRead(pin);
    return filter.biquadLPFApply(map(pressure, voltageMin, voltageMax, pressureMin, pressureMax));
}

void PressureObj::startPressureCheckThread() {
    this->checkPressureThread = new std::thread(&RunnablePressure::runThread, this);
    this->threadID = this->checkPressureThread->get_id();
    threads.setTimeSlice(threadID, this->sample_time);
    this->checkPressureThread->detach();
}
