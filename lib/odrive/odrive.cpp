/*
    * odrive.cpp
    *
    *  Created on: 2021-07-10 by Vincent Palmerio
    * 
    *  Note: Some functions have been copied over and modified from the ODriveArduino UART lib 
    *  from https://github.com/odriverobotics/ODriveArduino/blob/master/src/ODriveUART.cpp
*/

#include "odrive.h"
#include "router.h"

void ODriveController::setupODrives()
{
    LOX_ODRIVE_SERIAL.begin(LOX_ODRIVE_SERIAL_RATE);
    FUEL_ODRIVE_SERIAL.begin(FUEL_ODRIVE_SERIAL_RATE);

    while (getODriveState(LOX_ODRIVE_SERIAL) == AXIS_STATE_UNDEFINED) {
        Router::send("Waiting to connect to lox odrive...");
        delay(100);
    }

    while (getODriveState(FUEL_ODRIVE_SERIAL) == AXIS_STATE_UNDEFINED) {
        Router::send("Waiting to connect to fuel odrive...");
        delay(100);
    }

}

void ODriveController::setFuelODrivePosition(float position)
{
    fuelODrive.setPosition(0, position);
}

void ODriveController::setLOXODrivePosition(float position)
{
    loxODrive.setPosition(0, position);
}

AxisState ODriveController::getODriveState(HardwareSerial &serial) {
    return (AxisState) getParameterAsInt("axis0.current_state", serial);
}

//will return 0 if it cannot convert the message to int or no message was received
int ODriveController::getParameterAsInt(const String& path, HardwareSerial &serial) {
    return getParameterAsString(path, serial).toInt();
}

//will return 0.0 if it cannot convert the message to float or no message was received
float ODriveController::getParameterAsFloat(const String& path, HardwareSerial &serial) {
    return getParameterAsString(path, serial).toFloat();
}

String ODriveController::getParameterAsString(const String& path, HardwareSerial &serial) {
    serial.println("r " + path);
    return readLine(ODRIVE_COMM_TIMEOUT, serial);
}

String ODriveController::readLine(unsigned long timeout_ms, HardwareSerial &serial) {
    String str = "";
    unsigned long timeout_start = millis();
    for (;;) {
        while (!serial.available()) {
            if (millis() - timeout_start >= timeout_ms) {
                return str;
            }
        }
        char c = serial.read();
        if (c == '\n')
            break;
        str += c;
    }
    return str;
}