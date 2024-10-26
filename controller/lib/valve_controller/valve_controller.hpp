#ifndef VALVE_CONTROLLER_H
#define VALVE_CONTROLLER_H

/*
 * valve_controller.hpp
 *
 *  Created on: 2024-10-11 by Robert Nies
 *  Description: Code for open loop valve control
 */

void open_loop_thrust_control(float thrust, float ox_tank_pressure, float ipa_tank_pressure, float *angle_ox, float *angle_fuel);
void open_loop_thrust_control_defaults(float thrust, float *angle_ox, float *angle_fuel);
#endif