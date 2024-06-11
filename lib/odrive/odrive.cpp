/*
    * odrive.cpp
    *
    *  Created on: 2021-07-10 by Vincent Palmerio
    * 
*/

#include "odrive.h"
#include "router.h"

void ODriveController::setupODrives()
{
    LOX_ODRIVE_SERIAL.begin(LOX_ODRIVE_SERIAL_RATE);
    FUEL_ODRIVE_SERIAL.begin(FUEL_ODRIVE_SERIAL_RATE);

    while (loxODrive.getState() == AXIS_STATE_UNDEFINED) {
        Router::send("Waiting to connect to lox odrive...");
        delay(100);
    }

    while (fuelODrive.getState() == AXIS_STATE_UNDEFINED) {
        Router::send("Waiting to connect to fuel odrive...");
        delay(100);
    }

    Router::send("Setting lox odrive to closed loop control...");
    while (loxODrive.getState() != AXIS_STATE_CLOSED_LOOP_CONTROL) {
        loxODrive.clearErrors();
        loxODrive.setState(AXIS_STATE_CLOSED_LOOP_CONTROL);
        delay(10);
    }
   
    Router::send("Setting fuel odrive to closed loop control...");
    while (fuelODrive.getState() != AXIS_STATE_CLOSED_LOOP_CONTROL) {
        fuelODrive.clearErrors();

        fuelODrive.setState(AXIS_STATE_CLOSED_LOOP_CONTROL);
        delay(10);
    }

}