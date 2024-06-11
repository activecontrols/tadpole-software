#ifndef ODRIVE_H
#define ODRIVE_H

#include <ODriveArduino.h>
#include <loader.h>
#include <Wire.h>

#define LOX_ODRIVE_SERIAL Serial1
#define FUEL_ODRIVE_SERIAL Serial2

#define LOX_ODRIVE_SERIAL_RATE 115200
#define FUEL_ODRIVE_SERIAL_RATE 115200

#define ODRIVE_COMM_TIMEOUT 10 //in millis

namespace ODriveController {

    ODriveArduino loxODrive(LOX_ODRIVE_SERIAL);
    ODriveArduino fuelODrive(FUEL_ODRIVE_SERIAL);

    const int odriveMotor = 0;

    curve_header curveHeader;

    void* lerpPoints;

    extern void setupODrives();
    extern void setFuelODrivePosition(float position);
    extern void setLOXODrivePosition(float position);
    extern char* getODriveDataCSV();

    extern void setCurveHeader(curve_header ch);
    extern void setLerpPoints(void *points);

    extern void followThrottleCurve();
    extern void followSineCurve();
    extern void followChirpCurve();

    extern void clearErrors();

    extern void setThrust(float thrust);

    AxisState getODriveState(HardwareSerial &serial);
    String getParameterAsString(const String& path, HardwareSerial &serial);
    int getParameterAsInt(const String& path, HardwareSerial &serial);
    float getParameterAsFloat(const String& path, HardwareSerial &serial);
    String readLine(unsigned long timeout_ms, HardwareSerial &serial);

}

#endif