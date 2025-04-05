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

struct Fluid_Line {
  float tank_pressure;             // psi
  float venturi_upstream_pressure; // psi
  float venturi_throat_pressure;   // psi
  float temperature;               // K
};

struct Sensor_Data {
  Fluid_Line water;
};

struct FF_State {
  float ol_water_angle;
};

extern FF_State ff_state;
float open_loop_water_flow(float mass_flow_water, Sensor_Data sensor_data);
float closed_loop_water_flow(float mass_flow_water, Sensor_Data sensor_data);
#endif