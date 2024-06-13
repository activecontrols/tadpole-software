/*
    * Driver.cpp
    *
    *  Created on: 2021-07-10 by Vincent Palmerio
    *  Description: This file contains the implementation of the Driver namespace, which provides functions for controlling ODrive motors.
    *               It includes functions for logging telemetry data, creating log files, and following different types of curves.
    *               The curves supported are lerp (linear interpolation), sine, and chirp.
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

        /**
         * Logs the telemetry data for a curve in CSV format.
         * @param time The elapsed time in seconds.
         * @param phase The current phase of the curve.
         * @param thrust The thrust value (for closed lerp curves) or -1 (for other curve types).
         * @param lox_pos The LOX position.
         * @param ipa_pos The IPA position.
         */
        void logCurveTelemCSV(float time, int phase, float thrust, float lox_pos, float ipa_pos) {
            std::stringstream ss;
            ss << "," << time << "," << phase << "," << thrust << "," << lox_pos << "," << ipa_pos << getODriveStatusCSV();
            std::string csvRow = ss.str();
            Router::info(csvRow);
            odriveLogFile.println(csvRow.c_str());
        }

        /**
         * Creates a log file for the current curve.
         * @return The created log file.
         */
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

        /**
         * Performs linear interpolation between two values.
         * @param a The starting value.
         * @param b The ending value.
         * @param t0 The starting time.
         * @param t1 The ending time.
         * @param t The current time.
         * @return The interpolated value at the current time.
         */
        float lerp(float a, float b, float t0, float t1, float t) {
            if (t <= t0) return a;
            if (t >= t1) return b;
            if (t0 == t1) return b; // immediately get to b
            return a + (b - a) * ((t - t0) / (t1 - t0));
        }

        /**
         * Follows an open lerp curve by interpolating between LOX and IPA positions.
         */
        void followOpenLerpCurve() {
            lerp_point_open *los = Loader::los;

            elapsedMillis timer = elapsedMillis();
            unsigned long lastlog = timer;

            for (int i = 0; i < Loader::header.lerp.num_points - 1; i++) {
                while (timer / 1000.0 < los[i].time) {
                    float seconds = timer / 1000.0;
                    float lox_pos = lerp(los[i].lox_angle, los[i + 1].lox_angle, los[i].time, los[i + 1].time, seconds);
                    float ipa_pos = lerp(los[i].ipa_angle, los[i + 1].ipa_angle, los[i].time, los[i + 1].time, seconds);
                    setLOXPos(lox_pos);
                    setIPAPos(ipa_pos);
                    if (timer - lastlog > LOG_INTERVAL_MS) { // log every 10ms
                        logCurveTelemCSV(seconds, i, -1, lox_pos, ipa_pos);
                        lastlog = timer;
                    }
                    delay(COMMAND_INTERVAL_MS);
                }
            }
        }

        /**
         * Follows a closed lerp curve by interpolating between thrust values.
         */
        void followClosedLerpCurve() {
            lerp_point_closed *lcs = Loader::lcs;
            elapsedMillis timer = elapsedMillis();
            unsigned long lastlog = timer;

            for (int i = 0; i < Loader::header.lerp.num_points-1; i++) {
                while (timer / 1000.0 < lcs[i].time) {
                    float seconds = timer / 1000.0;
                    float thrust = lerp(lcs[i].thrust, lcs[i + 1].thrust, lcs[i].time, lcs[i + 1].time, seconds);
                    auto odrive_pos = setThrust(thrust);
                    if (timer - lastlog >= LOG_INTERVAL_MS) {
                        logCurveTelemCSV(seconds, i, thrust, odrive_pos.first, odrive_pos.second);
                        lastlog = timer;
                    }
                    delay(COMMAND_INTERVAL_MS);
                }
            }
        }

        /**
         * Follows an open sine curve by generating LOX and IPA positions based on sine wave.
         */
        void followOpenSineCurve() {
            
            float amplitude = Loader::header.sine_open.ipa_amplitude;
            float period = Loader::header.sine_open.ipa_period;
            int num_cycles = Loader::header.sine_open.ipa_num_cycles;
            float ipa_mix_ratio = Loader::header.sine_open.ipa_mix_ratio;

            elapsedMillis timer = elapsedMillis();
            unsigned long lastlog = timer;
            for (int i = 0; i < num_cycles; i++) {
                while (timer / 1000.0 < period) {
                    float seconds = timer / 1000.0;
                    float ipa_pos = amplitude * sin(2 * M_PI * seconds / period);
                    float lox_pos = (ipa_pos / ipa_mix_ratio) * (abs(ipa_mix_ratio - 1));

                    setLOXPos(lox_pos);
                    setIPAPos(ipa_pos);
                    if (timer - lastlog >= LOG_INTERVAL_MS) { // log every 10ms
                        logCurveTelemCSV(seconds, i, -1, lox_pos, ipa_pos);
                        lastlog = timer;
                    }
                    delay(COMMAND_INTERVAL_MS);
                }
            }
        }

        /**
         * Follows a closed sine curve by generating thrust values based on sine wave.
         */
        void followClosedSineCurve() {

            float amplitude = Loader::header.sine_closed.amplitude;
            float period = Loader::header.sine_closed.period;
            int num_cycles = Loader::header.sine_open.ipa_num_cycles;

            elapsedMillis timer = elapsedMillis();
            unsigned long lastlog = timer;
            for (int i = 0; i < num_cycles; i++) {
                while (timer / 1000.0 < period) {
                    float seconds = timer / 1000.0;
                    
                    float thrust = amplitude * sin(2 * M_PI * seconds / period);
                    auto odrive_pos = setThrust(thrust);
                    if (timer - lastlog >= LOG_INTERVAL_MS) {
                        logCurveTelemCSV(seconds, i, thrust, odrive_pos.first, odrive_pos.second);
                        lastlog = timer;
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

        Router::add({Driver::setPosCmd, "set_odrive_pos"});
        Router::add({Driver::setThrustCmd, "set_thrust"});

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

    /**
     * Set the position for the odrive controlling IPA flow.
     * @param pos value to be sent to odrive (valid values are from -1 to 1).
     */
    void setIPAPos(float pos) {
        fuelODrive.setPosition(pos);
    }

    /**
     * Set the position for the odrive controlling LOX flow.
     * @param pos value to be sent to odrive (valid values are from -1 to 1).
     */
    void setLOXPos(float pos) {
        loxODrive.setPosition(pos);
    }

    /**
     * Command for the Router lib to change the position of the IPA or LOX ODrive manually.
     */
    void setPosCmd() {

        char odriveSel[1] = {'\0'};
        Router::info("LOX or IPA? (Type l or i)");
        Router::receive(odriveSel, 1);

        char posString[POSITION_BUFFER_SIZE] = {'\0'};
        Router::info("Position?");
        Router::receive(posString, POSITION_BUFFER_SIZE);

        float pos;
        int result = std::sscanf(posString, "%f", &pos);
        if (result != 1) {
            Router::info("Could not convert input to a float, not continuing");
            return;
        }

        if (pos < MIN_ODRIVE_POS || pos > MAX_ODRIVE_POS) {
            Router::info("Position outside defined range in code, not continuing");
            return;
        }

        switch (odriveSel[0]) {
            case 'l':
                setLOXPos(pos);
                break;
            case 'i':
                setIPAPos(pos);
                break;
            default:
                Router::info("Invalid odrive specified");
                break;
        }
    }

    /**
     * Command for the Router lib to change the thrust manually.
     */
    void setThrustCmd() {

        char posString[POSITION_BUFFER_SIZE] = {'\0'};
        Router::info("Thrust value?");
        Router::receive(posString, POSITION_BUFFER_SIZE);

        float pos;
        int result = std::sscanf(posString, "%f", &pos);
        if (result != 1) {
            Router::info("Could not convert input to a float, not continuing");
            return;
        }

        if (pos < MIN_TRHUST || pos > MAX_THRUST) {
            Router::info("Thrust outside defined range in code, not continuing");
            return;
        }

        auto odrivePos = setThrust(pos);

        stringstream ss;
        ss << "LOX pos: " << odrivePos.first << " IPA pos: " << odrivePos.second;

        Router::info(ss.str());
    }

    /*
     * Sets the thrust
     * Uses a closed loop control to set the angle positions of the odrives
     * using feedback from the pressue sensor. The function will set the odrive positions itself, and
     * returns a tuple of lox and fuel throttle positions.
     */
    std::pair<float, float> setThrust(float thrust) {
        // TODO: closed loop magic

        return std::make_pair(0, 0); //(lox position, ipa position)
    }

    /**
     * Clears errors for both the LOX and IPA odrives.
     */
    void clearErrors() {
        loxODrive.clearErrors();
        fuelODrive.clearErrors();
    }

    /**
     * Blinks the LED on the LOX ODrive for 5 seconds.
     */
    void idenfityLOXODrive() {
        Router::info("Identifying LOX ODrive for 5 seconds...");
        loxODrive.setParameter("identify", true);
        delay(5000);
        loxODrive.setParameter("identify", false);
        Router::info("Done");
    }

    /**
     * Blinks the LED on the IPA ODrive for 5 seconds.
     */
    void idenfityFuelODrive() {
        Router::info("Identifying LOX ODrive for 5 seconds...");
        fuelODrive.setParameter("identify", true);
        delay(5000);
        fuelODrive.setParameter("identify", false);
        Router::info("Done");
    }

    /**
     * Returns a CSV string containing both the IPA and LOX ODrive Telemetry information, in the following format:
     * lox_pos,ipa_pos,lox_vel,ipa_vel,lox_voltage,ipa_voltage,lox_current,ipa_current
     */
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

    /**
     * Returns a string containing the hardware and firmware major and minor versons of the IPA and LOX ODrives and whether
     * they are misconfigured or require a reboot.
     */
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

    /**
     * Initiates curve following based on the curve header loaded in Loader.cpp.
     */
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

        bool open = Loader::header.is_open;

        switch (Loader::header.ctype) {
            case curve_type::lerp:
                (open ? followOpenLerpCurve() : followClosedLerpCurve());
                break;
            case curve_type::sine:
                (open ? followOpenSineCurve() : followClosedSineCurve());
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
