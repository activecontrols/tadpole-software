#ifndef TC_SENSOR_H
#define TC_SENSOR_H

// See docs/Sensor File.md for info

#include "Adafruit_MAX31856.h"

class Thermocouple : Adafruit_MAX31856 {

public:
  Thermocouple(int demuxAddr);
  void begin(max31856_thermocoupletype_t type);

  // get temperature in F
  float getTemperature_F();

  // get temperature in F
  float getTemperature_Kelvin();
};

namespace TC {
void begin();

extern Thermocouple lox_venturi_temperature;
extern Thermocouple lox_valve_temperature;
} // namespace TC

#endif // TC_SENSOR_H