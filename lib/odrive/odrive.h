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

#define LOX_ODRIVE_SERIAL Serial1
#define FUEL_ODRIVE_SERIAL Serial2

#define LOX_ODRIVE_SERIAL_RATE 115200
#define FUEL_ODRIVE_SERIAL_RATE 115200

#define ODRIVE_COMM_TIMEOUT 10 //in millis

namespace ODriveController {

    extern void setupODrives();
    extern void setFuelODrivePosition(float position);
    extern void setLOXODrivePosition(float position);
    extern void setThrust(float thrust);
    extern void clearErrors();

    extern void idenfityLOXODrive(); //flashes LED on LOX ODrive
    extern void idenfityFuelODrive(); //flashes LED on fuel ODrive

    extern void logCurveTelemCSV(int time, lerp_point_open &point);
    extern char* getODriveDataCSV();
    extern char* getODriveInfo();

    extern void followCurve();
    void followOpenLerpCurve();
    void followClosedLerpCurve();
    void followSineCurve();
    void followChirpCurve();

}

#endif