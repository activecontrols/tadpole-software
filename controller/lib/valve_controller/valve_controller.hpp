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
};

struct Fluid_Line {
  float tank_pressure;             // psi
  float venturi_upstream_pressure; // psi
  float venturi_throat_pressure;   // psi
  float temperature;               // K
};

struct Sensor_Data {
  float chamber_pressure; // psi
  Fluid_Line ox;
  Fluid_Line ipa;
};

void open_loop_thrust_control(float thrust, Sensor_Data sensor_data, float *angle_ox, float *angle_ipa);
void open_loop_thrust_control_defaults(float thrust, float *angle_ox, float *angle_ipa);
void closed_loop_thrust_control(float thrust, Sensor_Data sensor_data, float *angle_ox, float *angle_ipa);
#endif