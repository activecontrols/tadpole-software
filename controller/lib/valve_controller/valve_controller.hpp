#ifndef VALVE_CONTROLLER_H
#define VALVE_CONTROLLER_H

/*
 * valve_controller.hpp
 *
 *  Created on: 2024-10-11 by Robert Nies
 *  Description: Code for open loop valve control
 */

struct cav_vent {
  float cav_vent_throat_area; // in^2
  float cav_vent_cd;          // unitless
};

struct venturi {
  float inlet_area;  // in^2
  float outlet_area; // in^2
};

struct fluid {
  float vapour_pressure; // psi
  float density;         // lb / in^3
};

struct sensor_data {
  float chamber_pressure;      // psi
  float ox_tank_pressure;      // psi
  float ipa_tank_pressure;     // psi
  float ox_valve_temperature;  // K
  float ipa_valve_temperature; // K
  float ox_cv_temperature;     // K
  float ipa_cv_temperature;    // K
};

void open_loop_thrust_control(float thrust, sensor_data current_sensor_data, float *angle_ox, float *angle_fuel);
void open_loop_thrust_control_defaults(float thrust, float *angle_ox, float *angle_fuel);
#endif