#ifndef VALVE_CONTROLLER_H
#define VALVE_CONTROLLER_H

#include <math.h>
#define DEGREES_TO_RAD M_PI / 180 // usage 90 * DEGREES_TO_RAD

/*
 * valve_controller.hpp
 *
 *  Created on: 2024-10-11 by Robert Nies
 *  Description: Code for open loop valve control
 */

void open_loop_thrust_control(float thrust, float *angle_ox, float *angle_fuel);
#endif