/*
    * odrive.cpp
    *
    *  Created on: 2021-07-10 by Vincent Palmerio
    * 
*/

#include <sstream>
#include <SDCard.h>
#include <cmath>

#include "Driver.h"
#include "ODriveUART.h"

#define LOG_INTERVAL_MS 10
#define COMMAND_INTERVAL_MS 1

namespace Driver {

    ODriveUART loxODrive(LOX_ODRIVE_SERIAL);
    ODriveUART fuelODrive(FUEL_ODRIVE_SERIAL);

    namespace { // private

        File odriveLogFile;

        void logCurveTelemCSV(float time, int phase, float thrust, float lox_pos, float ipa_pos) {
            std::stringstream ss;
            ss << time << "," << phase << "," << thrust << "," << lox_pos << "," << ipa_pos << getODriveStatusCSV();
            std::string csvRow = ss.str();
            Router::info(csvRow);
            odriveLogFile.println(csvRow.c_str());
        }

        File createCurveLog() {
            std::stringstream ssfilename;
            ssfilename << Loader::header.curve_label;

            switch (Loader::header.ctype) {
                case curve_type::lerp:
                    ssfilename << (Loader::header.is_open ? "_lerp_open" : "_lerp_closed");
                    break;
                case curve_type::sine:
                    ssfilename << (Loader::header.is_open ? "_sine_open" : "_sine_closed");
                    break;
                case curve_type::chirp:
                    ssfilename << (Loader::header.is_open ? "_chirp_open" : "_chirp_closed");
                    break;
                default:
                    break;
            }
            ssfilename << ".csv";

            std::string filename = ssfilename.str();
            File odriveLogFile = SDCard::open(filename.c_str(), FILE_WRITE);

            if (!odriveLogFile) { // Failed to create a log file
                return odriveLogFile;
            }

            odriveLogFile.println(LOG_HEADER);

            return odriveLogFile;
        }

        float lerp(float a, float b, float t0, float t1, float t) {
            if (t <= t0) return a;
            if (t >= t1) return b;
            if (t0 == t1) return b; // immediately get to b
            return a + (b - a) * ((t - t0) / (t1 - t0));
        }

        void followLerpCurve() {
            lerp_point_open *los = Loader::los;
            lerp_point_closed *lcs = Loader::lcs;
            bool open = Loader::header.is_open;

            elapsedMillis timer = elapsedMillis();
            unsigned long lastlog = timer;

            for (int i = 0; i < Loader::header.lerp.num_points-1; i++) {
                while (timer / 1000.0 < (open ? los[i].time : lcs[i].time)) {
                    float seconds = timer / 1000.0;
                    if (open) {
                        float lox_pos = lerp(los[i].lox_angle, los[i + 1].lox_angle, los[i].time, los[i + 1].time, seconds);
                        float ipa_pos = lerp(los[i].ipa_angle, los[i + 1].ipa_angle, los[i].time, los[i + 1].time, seconds);
                        setLOXPos(lox_pos);
                        setIPAPos(ipa_pos);
                        if (timer - lastlog > LOG_INTERVAL_MS) { // log every 10ms
                            logCurveTelemCSV(seconds, i, -1, lox_pos, ipa_pos);
                            lastlog = timer;
                        }
                    } else { // closed
                        float thrust = lerp(lcs[i].thrust, lcs[i + 1].thrust, lcs[i].time, lcs[i + 1].time, seconds);
                        setThrust(thrust);
                        if (timer - lastlog >= LOG_INTERVAL_MS) {
                            logCurveTelemCSV(seconds, i, thrust, -1, -1);
                            lastlog = timer;
                        }
                    }
                    delay(COMMAND_INTERVAL_MS);
                }
            }
            Router::info("Finished following curve!");
        }

        void followSineCurve() {
            

            float amplitude = Loader::header.sine.amplitude;
            float period = Loader::header.sine.period;

            elapsedMillis timer = elapsedMillis();
            unsigned long lastlog = timer;
            for (int i = 0; i < Loader::header.sine.num_cycles; i++) {
                while (timer / 1000.0 < period) {
                    float seconds = timer / 1000.0;
                    if (Loader::header.is_open) {
                        float lox_pos = amplitude * sin(2 * M_PI * seconds / period);
                        float ipa_pos = amplitude * sin(2 * M_PI * seconds / period);
                        setLOXPos(lox_pos);
                        setIPAPos(ipa_pos);
                        if (timer - lastlog >= LOG_INTERVAL_MS) { // log every 10ms
                            logCurveTelemCSV(seconds, i, -1, lox_pos, ipa_pos);
                            lastlog = timer;
                        }
                    } else {
                        float thrust = amplitude * sin(2 * M_PI * seconds / period);
                        setThrust(thrust);
                        if (timer - lastlog > LOG_INTERVAL_MS) {
                            logCurveTelemCSV(seconds, i, thrust, -1, -1);
                            lastlog = timer;
                        }
                    }
                    delay(COMMAND_INTERVAL_MS);
                }
            }
        }

        void followChirpCurve() {
            

        }
    }

    void begin() {
        Router::add({Driver::followCurve, "follow_curve"});
        Router::add({Driver::idenfityLOXODrive, "identify_lox_odrive"});
        Router::add({Driver::idenfityFuelODrive, "identify_fuel_odrive"});
        Router::add({Driver::clearErrors, "clear_odrive_errors"});
        Router::add({Driver::printODriveInfo, "get_odrive_info"});
        Router::add({Driver::printODriveStatus, "get_odrive_status"});

        // Router::add({Driver::setIPAPos, "set_ipa_pos"});
        // Router::add({Driver::setLOXPos, "set_lox_pos"});
        // Router::add({Driver::setThrust, "set_thrust"});
        
        //modify router to allow parameters

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

    void setIPAPos(float pos) {
        fuelODrive.setPosition(pos);
    }

    void setLOXPos(float pos) {
        loxODrive.setPosition(pos);
    }

    void setThrust(float thrust) {
        // TODO: closed loop magic
    }

    void clearErrors() {
        loxODrive.clearErrors();
        fuelODrive.clearErrors();
    }

    void idenfityLOXODrive() {
        Router::info("Identifying LOX ODrive for 5 seconds...");
        loxODrive.setParameter("identify", true);
        delay(5000);
        loxODrive.setParameter("identify", false);
        Router::info("Done");
    }

    void idenfityFuelODrive() {
        Router::info("Identifying LOX ODrive for 5 seconds...");
        fuelODrive.setParameter("identify", true);
        delay(5000);
        fuelODrive.setParameter("identify", false);
        Router::info("Done");
    }

    std::string getODriveStatusCSV() {
        float loxThrottlePos = loxODrive.getPosition();
        float fuelThrottlePos = fuelODrive.getPosition();

        float loxThrottleVel = loxODrive.getVelocity();
        float fuelThrottleVel = fuelODrive.getVelocity();

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
           << " | LOX Reboot Required: " << loxRebootRequired << " ||| ";
        ss << "Fuel ODrive Hardware Version: " << fuelHWVersionMajor << "." << fuelHWVersionMinor
           << " | Fuel Firmware Version: " << fuelFWVersionMajor << "." << fuelFWVersionMinor
           << " | Fuel Misconfigured: " << fuelMisconfigured
           << " | Fuel Reboot Required: " << fuelRebootRequired << "\n";

        return ss.str();
    }

    void followCurve() {
        if (!Loader::loaded_curve) {
            Router::info("No curve loaded.");
            return;
        }

        odriveLogFile = createCurveLog();

        if (!odriveLogFile) {
            Router::info("Failed to create log file. Aborting.");
            return;
        }

        switch (Loader::header.ctype) {
            case curve_type::lerp:
                followLerpCurve();
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

        odriveLogFile.flush();
        odriveLogFile.close();

        Router::info("Finished following curve!");
    }
}
