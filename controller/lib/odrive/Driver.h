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


namespace Driver {

    void begin();

    void setPosCmd(); //no need
    void setThrust(float thrust); //no need
    void setThrustCmd();// no need
    void clearErrors(); //ported, needs new commands

    std::string getODriveStatusCSV(); //ported, needs modification, or modify log function
    inline void printODriveStatus() { //modify?? get rid of and add new commands?
        Router::info(LOG_HEADER);
        Router::info(getODriveStatusCSV()); 
    }
    void printODriveInfo();
    
    void followCurve();
    
};

#endif