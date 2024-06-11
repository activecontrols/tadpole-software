/*
    * odrive.cpp
    *
    *  Created on: 2021-07-10 by Vincent Palmerio
    * 
*/

#include <Router.h>
#include <Loader.h>

#include "odrive.h"

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

void ODriveController::setFuelODrivePosition(float position)
{
    fuelODrive.setPosition(position);
}

void ODriveController::setLOXODrivePosition(float position)
{
    loxODrive.setPosition(position);
}

void ODriveController::clearErrors()
{
    loxODrive.clearErrors();
    fuelODrive.clearErrors();
}

void ODriveController::followCurve()
{
    switch (Loader::header.ctype) {
        case curve_type::lerp:
            if (Loader::header.lerp.is_open) {
                followOpenLerpCurve();
            } else {
                followClosedLerpCurve();
            }
            break;
        case curve_type::sine:
            followSineCurve();
            break;
        case curve_type::chirp:
            followChirpCurve();
            break;
        default:
            break;
    }
}

void ODriveController::followOpenLerpCurve()
{
    lerp_point_open *point = Loader::los;
    elapsedMillis timer = elapsedMillis();
    for (int i = 0; i < Loader::header.lerp.num_points; i++) {

        setLOXODrivePosition(point[i].lox_angle);
        setFuelODrivePosition(point[i].ipa_angle);

        while (timer < point[i].time) {
            loxODrive.getPosition();
            fuelODrive.getPosition();

            logODriveDataCSV();
        }
    }

    Router::send("Finished following curve!");

}

