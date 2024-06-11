/*
    * odrive.h
    *
    *  Created on: 2021-07-10 by Vincent Palmerio
    * 
*/


#ifndef ODRIVE_H
#define ODRIVE_H

#include <Wire.h>

#include "ODriveUART.h"

#define LOX_ODRIVE_SERIAL Serial1
#define FUEL_ODRIVE_SERIAL Serial2

#define LOX_ODRIVE_SERIAL_RATE 115200
#define FUEL_ODRIVE_SERIAL_RATE 115200

#define ODRIVE_COMM_TIMEOUT 10 //in millis

namespace ODriveController {

    ODriveUART loxODrive(LOX_ODRIVE_SERIAL);
    ODriveUART fuelODrive(FUEL_ODRIVE_SERIAL);

    extern void setupODrives();
    extern void setFuelODrivePosition(float position);
    extern void setLOXODrivePosition(float position);
    extern void setThrust(float thrust);
    extern void clearErrors();

    extern void logODriveDataCSV();
    extern char* getODriveDataCSV();

    extern void followCurve();
    void followOpenLerpCurve();
    void followClosedLerpCurve();
    void followSineCurve();
    void followChirpCurve();

}

#endif