/*
    * Driver.h
    *
    *  Created on: 2024-06-10 by Vincent Palmerio
    *  Maintained by Vincent Palmerio and Ishan Goel
    *  Description: This file contains the declaration of the Driver namespace, which provides functions for 
    *  controlling the position and thrust of, and retrieviing info from, the LOX (Liquid Oxygen) and IPA (Isopropyl Alcohol) 
    *  ODrive valves in the Tadpole rocket engine. 
*/

#ifndef ODRIVE_H
#define ODRIVE_H

#include <Wire.h>
#include <Loader.h>
#include <Router.h>
#include <string>

#define LOX_ODRIVE_SERIAL Serial1
#define FUEL_ODRIVE_SERIAL Serial2

#define LOX_ODRIVE_SERIAL_RATE 115200
#define FUEL_ODRIVE_SERIAL_RATE 115200

#define LOG_HEADER "time,phase,thrust_cmd" \
                   "lox_pos_cmd,ipa_pos_cmd," \
                   "lox_pos,ipa_pos," \
                   "lox_vel,ipa_vel," \
                   "lox_voltage,ipa_voltage," \
                   "lox_current,ipa_current"

#define INT_BUFFER_SIZE 50
#define MAX_THRUST 100
#define MIN_TRHUST 0
#define MAX_ODRIVE_POS 1
#define MIN_ODRIVE_POS -1

#define ENABLE_ODRIVE_COMM false

namespace Driver {

    void begin();

    void setIPAPos(float position);
    void setLOXPos(float position);
    void setPosCmd();
    std::pair<float, float> setThrust(float thrust);
    void setThrustCmd();
    void clearErrors();

    void idenfityLOXODrive(); // flashes LED on LOX ODrive
    void idenfityFuelODrive(); // flashes LED on fuel ODrive

    std::string getODriveStatusCSV();
    std::string getODriveInfo();
    inline void printODriveStatus() { Router::info(getODriveStatusCSV()); }
    inline void printODriveInfo() { Router::info(getODriveInfo()); }

    void followCurve();
    
};

#endif