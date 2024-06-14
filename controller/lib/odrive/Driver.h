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

#define POSITION_BUFFER_SIZE 50
#define MAX_THRUST 100
#define MIN_TRHUST 0
#define MAX_ODRIVE_POS 1
#define MIN_ODRIVE_POS -1

#define ENABLE_ODRIVE_COMM true

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