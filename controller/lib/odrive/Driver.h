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

#include <Loader.h>
#include <Router.h>
#include <string>

#define LOX_ODRIVE_SERIAL (Serial1)
#define IPA_ODRIVE_SERIAL (Serial2)

#define LOX_ODRIVE_SERIAL_RATE (115200)
#define IPA_ODRIVE_SERIAL_RATE (115200)

#define LOG_HEADER ("time,phase,thrust_cmd," \
                   "lox_pos_cmd,ipa_pos_cmd," \
                   "lox_pos,lox_vel," \
                   "lox_voltage,lox_current," \
                   "ipa_pos,ipa_vel," \
                   "ipa_voltage,ipa_current")


namespace Driver {

    void begin();

    void setThrust(float thrust);
    void setThrustCmd();

    void printODriveInfo();
    
    void followCurve();
    bool watchdogThreadsEnded();
    
};

#endif