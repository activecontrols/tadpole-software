#ifndef TC_SENSOR_H
#define TC_SENSOR_H

/*
 * Thermocouples.h
 *
 *  Created on: 2025-7-02 by Robert Nies
 *  Description: This file contains the declaration of the Thermocouple class,
 *  in which each object is used to read from a specific thermocouple.
 */

class Thermocouple {

public:
  Thermocouple(int demuxAddr);
  float getTemperature();
};

namespace TC {
extern Thermocouple example_tc;
} // namespace TC

#endif // TC_SENSOR_H