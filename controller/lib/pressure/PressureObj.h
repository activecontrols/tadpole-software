#ifndef PRSSUREOBJ_H
#define PRSSUREOBJ_H

/*
    * Pressure.h
    *
    *  Created on: 2024-6-19 by Vincent Palmerio
    *  Description: This file contains the declaration of the PressureObj class, 
    *  in which each object is used to read from a specific pressure sensor.
*/

class PressureObj {

private:
    unsigned int pin;
    float pressureMin;
    float pressureMax;
    float voltageMin;
    float voltageMax;

public:
    PressureObj(unsigned int pin, float pressureMin, float pressureMax, float voltageMin, float voltageMax);
    float getPressure();

};


#endif // PRSSUREOBJ_H