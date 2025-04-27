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
  float valve_upstream_pressure;       // psi
  float valve_downstream_pressure;     // psi
  float venturi_differential_pressure; // psi
  float venturi_temperature;           // K
  float valve_temperature;             // K
};

struct Sensor_Data {
  float chamber_pressure; // psi
  Fluid_Line ox;
  Fluid_Line ipa;
};

struct VC_State {
  float ol_lox_mdot;
  float ol_ipa_mdot;
  float measured_lox_mdot;
  float measured_ipa_mdot;
  float ol_lox_angle;
  float ol_ipa_angle;
};

extern VC_State vc_state;
void open_loop_thrust_control(float thrust, Sensor_Data sensor_data, float *angle_ox, float *angle_ipa);
void closed_loop_thrust_control(float thrust, Sensor_Data sensor_data, float *angle_ox, float *angle_ipa);
void log_only(Sensor_Data sensor_data);
#endif