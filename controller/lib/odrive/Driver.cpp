/*
    * odrive.cpp
    *
    *  Created on: 2021-07-10 by Vincent Palmerio
    * 
*/

#include <sstream>
#include <Router.h>
#include <SDCard.h>

#include "Driver.h"
#include "ODriveUART.h"

namespace Driver {

    ODriveUART loxODrive(LOX_ODRIVE_SERIAL);
    ODriveUART fuelODrive(FUEL_ODRIVE_SERIAL);

    namespace { // private

        void logCurveTelemCSV(File *file, unsigned long time, int pointNum, lerp_point_open &point) {
            std::string csvRowODriveData = getODriveDataCSV();

            std::stringstream ss;
            ss << "," << time << "," << pointNum << point.lox_angle << "," << point.ipa_angle << ","
               << csvRowODriveData;
            std::string csvRow = ss.str();
            Router::info(csvRow);

            file->println(csvRow.c_str());
        }

        File createCurveLog() {
            std::stringstream ss;
            ss << Loader::header.curve_label;

            const char *csvHeader = nullptr;
            switch (Loader::header.ctype) {
                case curve_type::lerp:
                    if (Loader::header.lerp.is_open) {
                        ss << "_lerp_open";
                        csvHeader = ODRIVE_OPEN_CURVE_HEADER;
                    } else {
                        ss << "_lerp_closed";
                        csvHeader = ODRIVE_CLOSED_CURVE_HEADER;
                    }
                    break;
                case curve_type::sine:
                    ss << "_sine_closed";
                    csvHeader = ODRIVE_CLOSED_CURVE_HEADER;
                    break;
                case curve_type::chirp:
                    ss << "_chirp";
                    csvHeader = ODRIVE_CLOSED_CURVE_HEADER;
                    break;
                default:
                    break;
            }
            ss << ".csv";

            std::string filename = ss.str();
            File odriveLogFile = SDCard::open(filename.c_str(), 'w');

            odriveLogFile.println(csvHeader);
            odriveLogFile.println(getODriveDataCSV().c_str());

            return odriveLogFile;
        }

        void followOpenLerpCurve() {
            File odriveLogFile = createCurveLog();
            lerp_point_open *point = Loader::los;
            elapsedMillis timer = elapsedMillis();

            for (int i = 0; i < Loader::header.lerp.num_points; i++) {
                setLOXODrivePosition(point[i].lox_angle);
                setFuelODrivePosition(point[i].ipa_angle);

                while (timer / 1000.0 < point[i].time) {
                    logCurveTelemCSV(&odriveLogFile, timer, i, point[i]);
                }
            }
            Router::info("Finished following curve!");
        }

        void followClosedLerpCurve() {
            File odriveLogFile = createCurveLog();
            lerp_point_closed *point = Loader::lcs;
            elapsedMillis timer = elapsedMillis();

            for (int i = 0; i < Loader::header.lerp.num_points; i++) {
                //setThrust(point[i].thrust);

                //log thrust as it is changing by PID
                while (timer / 1000.0 < point[i].time) {
                    //logCurveTelemCSV(timer, point[i]);
                }
            }
            Router::info("Finished following curve!");
        }

        void followSineCurve() {
            File odriveLogFile = createCurveLog();
            //create sine curve
            //follow sine curve
            //log data
        }

        void followChirpCurve() {
            File odriveLogFile = createCurveLog();

            //create chirp curve
            //follow chirp curve
            //log data
        }
    }

    void begin() {
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

        Router::info(getODriveInfo());

    }

    void setFuelODrivePosition(float position) {
        fuelODrive.setPosition(position);
    }

    void setLOXODrivePosition(float position) {
        loxODrive.setPosition(position);
    }

    void clearErrors() {
        loxODrive.clearErrors();
        fuelODrive.clearErrors();
    }

    std::string getODriveDataCSV() {
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

    std::string getODriveInfo() {
        int loxHWVersionMajor = loxODrive.getParameterAsInt("hw_version_major");
        int loxHWVersionMinor = loxODrive.getParameterAsInt("hw_version_minor");
        int loxFWVersionMajor = loxODrive.getParameterAsInt("fw_version_major");
        int loxFWVersionMinor = loxODrive.getParameterAsInt("fw_version_minor");
        String loxMisconfigured = loxODrive.getParameterAsString("misconfigured");
        String loxRebootRequired = loxODrive.getParameterAsString("reboot_required");

        int fuelHWVersionMajor = fuelODrive.getParameterAsInt("hw_version_major");
        int fuelHWVersionMinor = fuelODrive.getParameterAsInt("hw_version_minor");
        int fuelFWVersionMajor = fuelODrive.getParameterAsInt("fw_version_major");
        int fuelFWVersionMinor = fuelODrive.getParameterAsInt("fw_version_minor");
        String fuelMisconfigured = fuelODrive.getParameterAsString("misconfigured");
        String fuelRebootRequired = fuelODrive.getParameterAsString("reboot_required");

        stringstream ss = stringstream();
        ss << "LOX ODrive Hardware Version: " << loxHWVersionMajor << "." << loxHWVersionMinor
           << " | LOX Firmware Version: " << loxFWVersionMajor << "." << loxFWVersionMinor
           << " | LOX Misconfigured: " << loxMisconfigured
           << " | LOX Reboot Required: " << loxRebootRequired << " ||| "
           << "Fuel ODrive Hardware Version: " << fuelHWVersionMajor << "." << fuelHWVersionMinor
           << " | Fuel Firmware Version: " << fuelFWVersionMajor << "." << fuelFWVersionMinor
           << " | Fuel Misconfigured: " << fuelMisconfigured
           << " | Fuel Reboot Required: " << fuelRebootRequired << "\n";

        return ss.str();
    }

    namespace {

        File odriveLogFile;

        void logCurveTelemCSV(unsigned long time, int pointNum, void *point) {
            std::string csvRowODriveData = getODriveDataCSV();

            std::stringstream ss;
            
            if (Loader::header.lerp.is_open) {
                lerp_point_open *openPoint = (lerp_point_open*) point;
                ss << "," << time << "," << pointNum << openPoint->lox_angle << "," << openPoint->ipa_angle << "," << csvRowODriveData;
            } else {
                lerp_point_closed *closedPoint = (lerp_point_closed*) point;
                ss << "," << time << "," << pointNum << closedPoint->thrust << "," << csvRowODriveData;
            }
            
            std::string csvRow = ss.str();
            Router::info(csvRow);

            odriveLogFile.println(csvRow.c_str());
        }

        File createCurveLog() {
            
            std::stringstream ss;
            ss << Loader::header.curve_label;

            const char *csvHeader = nullptr;
            switch (Loader::header.ctype) {
                case curve_type::lerp:
                    if (Loader::header.lerp.is_open) {
                        ss << "_lerp_open";
                        csvHeader = ODRIVE_OPEN_CURVE_HEADER;
                    } else {
                        ss << "_lerp_closed";
                        csvHeader = ODRIVE_CLOSED_CURVE_HEADER;
                    }
                    break;
                case curve_type::sine:
                    ss << "_sine_closed";
                    csvHeader = ODRIVE_CLOSED_CURVE_HEADER;
                    break;
                case curve_type::chirp:
                    ss << "_chirp";
                    csvHeader = ODRIVE_CLOSED_CURVE_HEADER;
                    break;
                default:
                    break;
            }

            ss << ".csv";

            char *filename = (char*) ss.str().c_str();

            File odriveLogFile = SDCard::open(filename, 'w');

            odriveLogFile.println(csvHeader);

            odriveLogFile.println(getODriveDataCSV().c_str());

            return odriveLogFile;
        }

        void followOpenLerpCurve() {

            lerp_point_open *point = Loader::los;
            elapsedMillis timer = elapsedMillis();
            for (int i = 0; i < Loader::header.lerp.num_points; i++) {

                setLOXODrivePosition(point[i].lox_angle);
                setFuelODrivePosition(point[i].ipa_angle);

                while (timer/1000.0 < point[i].time) {
                    logCurveTelemCSV(timer, i, (void *) &point[i]);
                }
            }
            Router::info("Finished following curve!");
        }

        void followClosedLerpCurve() {
            
            lerp_point_closed *point = Loader::lcs;
            elapsedMillis timer = elapsedMillis();
            for (int i = 0; i < Loader::header.lerp.num_points; i++) {

                //setThrust(point[i].thrust); 
                
                //log thrust as it is changing by PID

                while (timer/1000.0 < point[i].time) {
                    logCurveTelemCSV(timer, i, (void *) &point[i]);
                }
            }
            Router::info("Finished following curve!");
        }

        void followSineCurve() {
            
            
            //create sine curve
            //follow sine curve
            //log data
        }

        void followChirpCurve() {
            
            

            //create chirp curve
            //follow chirp curve
            //log data
        }
    }

    void followCurve() {
        if (!Loader::loaded_curve) {
            Router::info("No curve loaded.");
            return;
        }

        odriveLogFile = createCurveLog();

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
}
