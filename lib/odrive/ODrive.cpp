/*
    * odrive.cpp
    *
    *  Created on: 2021-07-10 by Vincent Palmerio
    * 
*/

#include <sstream>
#include <string.h>
#include <Router.h>

#include "ODrive.h"
#include "ODriveUART.h"

ODriveUART loxODrive(LOX_ODRIVE_SERIAL);
ODriveUART fuelODrive(FUEL_ODRIVE_SERIAL);

void ODrive::begin() {
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

void ODrive::setFuelODrivePosition(float position) {
    fuelODrive.setPosition(position);
}

void ODrive::setLOXODrivePosition(float position) {
    loxODrive.setPosition(position);
}

void ODrive::clearErrors() {
    loxODrive.clearErrors();
    fuelODrive.clearErrors();
}

void ODrive::logCurveTelemCSV(int time, lerp_point_open &point) {
    char *csvRowODriveData = getODriveDataCSV();

    std::stringstream ss;
    ss << time << "," << point.lox_angle << "," << point.ipa_angle << "," << csvRowODriveData;

    const char *csvRowTelemetry = ss.str().c_str();

    Router::send(csvRowTelemetry);

    //log to sd card

    free(csvRowODriveData);
    csvRowODriveData = NULL;

}

char *ODrive::getODriveDataCSV() {
    int loxThrottlePos = loxODrive.getPosition();
    int fuelThrottlePos = fuelODrive.getPosition();

    int loxThrottleVel = loxODrive.getVelocity();
    int fuelThrottleVel = fuelODrive.getVelocity();

    float loxVoltage = loxODrive.getParameterAsFloat("vbus_voltage");
    float fuelVoltage = fuelODrive.getParameterAsFloat("vbus_voltage");

    float loxCurrent = loxODrive.getParameterAsFloat("ibus");
    float fuelCurrent = fuelODrive.getParameterAsFloat("ibus");

    std::stringstream ss;
    ss << loxThrottlePos << "," << fuelThrottlePos << ","
       << loxThrottleVel << "," << fuelThrottleVel << ","
       << loxVoltage << "," << fuelVoltage << ","
       << loxCurrent << "," << fuelCurrent;

    std::string csvString = ss.str();
    char *cstr = new char[csvString.length() + 1];
    strcpy(cstr, csvString.c_str());
    cstr[csvString.length()] = '\0';

    return cstr;
}

void ODrive::followCurve() {
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

void ODrive::followOpenLerpCurve() {
    //create file for logging csv data in sd card
        //include curve id in the csv header or file name
        //include curve type in file name
    //make the first line the csv header

    lerp_point_open *point = Loader::los;
    elapsedMillis timer = elapsedMillis();
    for (int i = 0; i < Loader::header.lerp.num_points; i++) {

        setLOXODrivePosition(point[i].lox_angle);
        setFuelODrivePosition(point[i].ipa_angle);

        //log timer and log point it is following? or log the number of the point it is following

        while (timer < point[i].time) {
            logCurveTelemCSV(timer, point[i]);
        }
    }

    Router::send("Finished following curve!");

}

