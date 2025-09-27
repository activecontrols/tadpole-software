#ifndef VALVE_CONTROLLER_H
#define VALVE_CONTROLLER_H

/*
 * valve_controller.hpp
 *
 *  Created on: 2024-10-11 by Robert Nies
 *  Description: Code for open loop valve control
 */

struct Venturi {
  float inlet_area;  // in^2
  float throat_area; // in^2
  float cd;
};

struct Sensor_Data {
  float water_venturi_upstream_pressure;
  float water_venturi_throat_pressure;
};

struct VC_State {
  float measured_water_mdot;
};

VC_State log_only(Sensor_Data sensor_data);
#endif