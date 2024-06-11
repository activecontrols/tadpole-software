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

class ODrive {
public:
    static void begin();
private:
    static void setFuelODrivePosition(float position);
    static void setLOXODrivePosition(float position);
    static void setThrust(float thrust);
    static void clearErrors();

    static void idenfityLOXODrive(); // flashes LED on LOX ODrive
    static void idenfityFuelODrive(); // flashes LED on fuel ODrive

    static void logCurveTelemCSV(int time, lerp_point_open &point);
    static char* getODriveDataCSV();
    static char* getODriveInfo();

    static void followCurve();
    static void followOpenLerpCurve();
    static void followClosedLerpCurve();
    static void followSineCurve();
    static void followChirpCurve();
};

#endif