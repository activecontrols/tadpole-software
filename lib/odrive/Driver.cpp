/*
    * odrive.cpp
    *
    *  Created on: 2021-07-10 by Vincent Palmerio
    * 
*/

#include <sstream>
#include <string>
#include <Router.h>

#include "Driver.h"
#include "ODriveUART.h"

ODriveUART loxODrive(LOX_ODRIVE_SERIAL);
ODriveUART fuelODrive(FUEL_ODRIVE_SERIAL);

void Driver::begin() {
    Router::add({Driver::followCurve, "follow_curve"});
    LOX_ODRIVE_SERIAL.begin(LOX_ODRIVE_SERIAL_RATE);
    FUEL_ODRIVE_SERIAL.begin(FUEL_ODRIVE_SERIAL_RATE);

    while (loxODrive.getState() == AXIS_STATE_UNDEFINED) {
        Router::info("Waiting to connect to lox odrive...");
        delay(100);
    }

    while (fuelODrive.getState() == AXIS_STATE_UNDEFINED) {
        Router::info("Waiting to connect to fuel odrive...");
        delay(100);
    }

    Router::info("Setting lox odrive to closed loop control...");
    while (loxODrive.getState() != AXIS_STATE_CLOSED_LOOP_CONTROL) {
        loxODrive.clearErrors();
        loxODrive.setState(AXIS_STATE_CLOSED_LOOP_CONTROL);
        delay(10);
    }

    Router::info("Setting fuel odrive to closed loop control...");
    while (fuelODrive.getState() != AXIS_STATE_CLOSED_LOOP_CONTROL) {
        fuelODrive.clearErrors();

        fuelODrive.setState(AXIS_STATE_CLOSED_LOOP_CONTROL);
        delay(10);
    }
}

void Driver::setFuelODrivePosition(float position) {
    fuelODrive.setPosition(position);
}

void Driver::setLOXODrivePosition(float position) {
    loxODrive.setPosition(position);
}

void Driver::clearErrors() {
    loxODrive.clearErrors();
    fuelODrive.clearErrors();
}

void Driver::logCurveTelemCSV(unsigned long time, lerp_point_open &point) {
    std::string csvRowODriveData = getODriveDataCSV();

    std::stringstream ss;
    ss << time << "," << point.lox_angle << "," << point.ipa_angle << "," << csvRowODriveData;
    Router::info(ss.str());

    //log to sd card
}

std::string Driver::getODriveDataCSV() {
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

    return ss.str();
}

void Driver::followCurve() {
    switch (Loader::header.ctype) {
        case curve_type::lerp:
            Loader::header.lerp.is_open ? followOpenLerpCurve() : followClosedLerpCurve();
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

void Driver::followOpenLerpCurve() {
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

        while (timer/1000.0 < point[i].time) {
            logCurveTelemCSV(timer, point[i]);
        }
    }
    Router::info("Finished following curve!");
}

