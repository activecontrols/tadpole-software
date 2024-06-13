/*
    * odrive.h
    *
    *  Created on: 2021-07-10 by Vincent Palmerio
    * 
*/

#ifndef ODRIVE_H
#define ODRIVE_H

#include <Wire.h>
#include <Loader.h>
#include <string>

#define LOX_ODRIVE_SERIAL Serial1
#define FUEL_ODRIVE_SERIAL Serial2

#define LOX_ODRIVE_SERIAL_RATE 115200
#define FUEL_ODRIVE_SERIAL_RATE 115200

//#define ODRIVE_OPEN_CURVE_HEADER "info,current_time,point_number," \
//                                  "point_lox_angle,point_ipa_angle," \
//                                  "lox_throttle_pos,ipa_throttle_pos," \
//                                  "lox_throttle_vel,ipa_throttle_vel," \
//                                  "lox_voltage,ipa_voltage," \
//                                  "lox_current,ipa_current"
//
//#define ODRIVE_CLOSED_CURVE_HEADER "info,current_time,point_number," \
//                                   "point_thrust," \
//                                   "lox_throttle_pos,ipa_throttle_pos," \
//                                   "lox_throttle_vel,ipa_throttle_vel," \
//                                   "lox_voltage,ipa_voltage," \
//                                   "lox_current,ipa_current"

#define LOG_HEADER "time,phase,thrust_cmd" \
                   "lox_pos_cmd,ipa_pos_cmd," \
                   "lox_pos,ipa_pos," \
                   "lox_vel,ipa_vel," \
                   "lox_voltage,ipa_voltage," \
                   "lox_current,ipa_current"

namespace Driver {

    void begin();

    void setIPAPos(float position);
    void setLOXPos(float position);
    void setThrust(float thrust);
    void clearErrors();

    void idenfityLOXODrive(); // flashes LED on LOX ODrive
    void idenfityFuelODrive(); // flashes LED on fuel ODrive

    std::string getODriveStatusCSV();
    std::string getODriveInfo();

    void followCurve();
    
};

#endif