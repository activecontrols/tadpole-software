/*
 * Driver.h
 *
 *  Created on: 2024-06-10 by Vincent Palmerio
 *  Maintained by Vincent Palmerio and Ishan Goel
 *  Description: This file contains the declaration of the Driver namespace, which provides functions for
 *  controlling the position and thrust of, and retrieviing info from, the LOX (Liquid Oxygen) and IPA (Isopropyl Alcohol)
 *  ODrive valves in the Tadpole rocket engine.
 */

#ifndef DRIVER_H
#define DRIVER_H

#include "Loader.h"
#include "Router.h"
#include "valve_controller.hpp"

#define DONT_KILL 0                     // continue loop
#define KILLED_BY_ZUCROW 1              // zucrow panic line caused kill
#define KILLED_BY_SERIAL 2              // serial cmd caused kill
#define KILLED_BY_ANGLE_OOR 3           // angle compare caused kill
#define ANGLE_OOR_THRESH (10.0 / 360.0) // if angle is 10 degrees off, instantly kill
#define KILLED_BY_ODRIVE_FAULT 4        // odrive exits closed loop control

#define LOG_HEADER ("time,phase,thrust_cmd,lox_pos_cmd,ipa_pos_cmd," \
                    "lox_pos,lox_vel,lox_voltage,lox_current,"       \
                    "ipa_pos,ipa_vel,ipa_voltage,ipa_current")

namespace Driver {

void begin();
void printODriveInfo();
void setThrustCmd();
void followCurve();
void followCurveCmd();
void createCurveLog(const char *filename);

}; // namespace Driver

#endif