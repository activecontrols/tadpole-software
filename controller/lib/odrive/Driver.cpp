/*
    * Driver.cpp
    *
    *  Created on: 2024-06-10 by Vincent Palmerio
    *  Maintained by Vincent Palmerio and Ishan Goel
    *  Description: This file contains the implementation of the Driver namespace, which provides functions for controlling ODrive motors.
    *               It includes functions for logging telemetry data, creating log files, and following different types of curves.
    *               The curves supported are lerp (linear interpolation), sine, and chirp.
    */

#include <sstream>
#include <Arduino.h>

#include "SDCard.h"
#include "Driver.h"
#include "ODriveUART.h"

#define LOG_INTERVAL_MS 10
#define COMMAND_INTERVAL_MS 1

namespace Driver {

#if (ENABLE_ODRIVE_COMM)
    ODriveUART loxODrive(LOX_ODRIVE_SERIAL);
    ODriveUART fuelODrive(FUEL_ODRIVE_SERIAL);
#endif

    namespace { // private

        File odriveLogFile;

        float loxPosCmd; //the last position command sent to the LOX odrive
        float ipaPosCmd; //the last position command sent to the IPA odrive

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
            if (Router::logenabled) {
                odriveLogFile.println(csvRow.c_str());
            }
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
                    lox_pos = constrain(lox_pos, MIN_ODRIVE_POS, MAX_ODRIVE_POS);
                    ipa_pos = constrain(ipa_pos, MIN_ODRIVE_POS, MAX_ODRIVE_POS);
                    setLOXPos(lox_pos);
                    setIPAPos(ipa_pos);
                    if (timer - lastlog > LOG_INTERVAL_MS) {
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
                    setThrust(thrust);
                    if (timer - lastlog >= LOG_INTERVAL_MS) {
                        logCurveTelemCSV(seconds, i, thrust, loxPosCmd, ipaPosCmd);
                        lastlog = timer;
                    }
                    delay(COMMAND_INTERVAL_MS);
                }
            }
        }

        /**
         * Follows a closed or open sine curve by generating lox/ipa positions or thrust values.
         */
        void followSineCurve() {
            float amplitude = Loader::header.sine.amplitude;
            if (amplitude > 1.0 || amplitude < -1.0) {
                Router::info("|amplitude| > 1.0, saturating.");
                amplitude = amplitude > 0 ? 1.0 : -1.0;
            }
            float period = Loader::header.sine.period;
            int num_cycles = Loader::header.sine.num_cycles;

            elapsedMillis timer = elapsedMillis();
            unsigned long lastlog = timer;
            float ipa_pos, lox_pos, thrust = -1;

            for (int i = 0; i < num_cycles; i++) {
                while (timer / 1000.0 < period) {
                    float seconds = timer / 1000.0;
                    if (Loader::header.is_open) {
                        lox_pos = abs(amplitude * (sin(2 * M_PI * seconds / period) + 1.0) / 2.0);
                        ipa_pos = lox_pos / Loader::header.of_ratio;
                        lox_pos = constrain(lox_pos, MIN_ODRIVE_POS, MAX_ODRIVE_POS);
                        ipa_pos = constrain(ipa_pos, MIN_ODRIVE_POS, MAX_ODRIVE_POS);
                        setLOXPos(lox_pos);
                        setIPAPos(ipa_pos);
                    } else {
                        thrust = amplitude * (sin(2 * M_PI * seconds / period) + 1.0) / 2.0;
                        setThrust(thrust);
                    }
                    if (timer - lastlog >= LOG_INTERVAL_MS) {
                        logCurveTelemCSV(seconds, i, thrust, loxPosCmd, ipaPosCmd);
                        lastlog = timer;
                    }
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
        Router::add({Driver::printLOXCmdPos, "get_lox_cmd_pos"});
        Router::add({Driver::printIPACmdPos, "get_ipa_cmd_pos"});

        Router::add({Driver::setPosCmd, "set_odrive_pos"});
        Router::add({Driver::setThrustCmd, "set_thrust"});

#if (ENABLE_ODRIVE_COMM)
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
#endif

    }

    /**
     * Set the position for the odrive controlling IPA flow.
     * @param pos value to be sent to odrive (valid values are from -1 to 1).
     */
    void setIPAPos(float pos) {
#if (ENABLE_ODRIVE_COMM)
        fuelODrive.setPosition(pos);
#endif
        ipaPosCmd = pos;
    }

    /**
     * Set the position for the odrive controlling LOX flow.
     * @param pos value to be sent to odrive (valid values are from -1 to 1).
     */
    void setLOXPos(float pos) {
#if (ENABLE_ODRIVE_COMM)
        loxODrive.setPosition(pos);
#endif
        loxPosCmd = pos;
    }

    float getLOXCmdPos() {
        return loxPosCmd;
    }

    float getIPACmdPos() {
        return ipaPosCmd;
    }

    /**
     * Command for the Router lib to change the position of the IPA or LOX ODrive manually.
     */
    void setPosCmd() {

        Router::info("LOX or IPA? (Type l or i)");
        String odriveSel = Router::read(3);
        Router::info("Response: " + odriveSel);

        Router::info("Position?");
        String posString = Router::read(INT_BUFFER_SIZE);
        Router::info("Response: " + posString);

        float pos;
        int result = std::sscanf(posString.c_str(), "%f", &pos);
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
        Router::info("Position set");
    }

    /**
     * Command for the Router lib to change the thrust manually.
     */
    void setThrustCmd() {
        Router::info("Position?");
        String thrustString = Router::read(INT_BUFFER_SIZE);
        Router::info("Response: " + thrustString);

        float thrust;
        int result = std::sscanf(thrustString.c_str(), "%f", &thrust);
        if (result != 1) {
            Router::info("Could not convert input to a float, not continuing");
            return;
        }

        if (thrust < MIN_TRHUST || thrust > MAX_THRUST) {
            Router::info("Thrust outside defined range in code, not continuing");
            return;
        }

        setThrust(thrust);

        stringstream ss;
        ss << "Thrust set. LOX pos: " << loxPosCmd << " IPA pos: " << ipaPosCmd;

        Router::info(ss.str());
    }

    /*
     * Sets the thrust
     * Uses a closed loop control to set the angle positions of the odrives
     * using feedback from the pressue sensor. The function will set the odrive positions itself, and
     * modify currentLOXPos and currentIPAPos to what it set the odrive positions to.
     */
    void setThrust(float thrust) {
        // TODO: closed loop magic
        ipaPosCmd = 0.0;
        loxPosCmd = 0.0;
    }

    /**
     * Clears errors for both the LOX and IPA odrives.
     */
    void clearErrors() {
#if (ENABLE_ODRIVE_COMM)
        loxODrive.clearErrors();
        fuelODrive.clearErrors();
#endif
    }

    /**
     * Blinks the LED on the LOX ODrive for 5 seconds.
     */
    void idenfityLOXODrive() {
        Router::info("Identifying LOX ODrive for 5 seconds...");
#if (ENABLE_ODRIVE_COMM)
        loxODrive.setParameter("identify", true);
        delay(5000);
        loxODrive.setParameter("identify", false);
#endif
        Router::info("Done");
    }

    /**
     * Blinks the LED on the IPA ODrive for 5 seconds.
     */
    void idenfityFuelODrive() {
        Router::info("Identifying LOX ODrive for 5 seconds...");
#if (ENABLE_ODRIVE_COMM)
        fuelODrive.setParameter("identify", true);
        delay(5000);
        fuelODrive.setParameter("identify", false);
#endif
        Router::info("Done");
    }

    /**
     * Returns a CSV string containing both the IPA and LOX ODrive Telemetry information, in the following format:
     * lox_pos,ipa_pos,lox_vel,ipa_vel,lox_voltage,ipa_voltage,lox_current,ipa_current
     */
    std::string getODriveStatusCSV() {
        std::stringstream ss;
#if (ENABLE_ODRIVE_COMM)
        float loxThrottlePos = loxODrive.getPosition();
        float fuelThrottlePos = fuelODrive.getPosition();

        float loxThrottleVel = loxODrive.getVelocity();
        float fuelThrottleVel = fuelODrive.getVelocity();

        float loxVoltage = loxODrive.getParameterAsFloat("vbus_voltage");
        float fuelVoltage = fuelODrive.getParameterAsFloat("vbus_voltage");

        float loxCurrent = loxODrive.getParameterAsFloat("ibus");
        float fuelCurrent = fuelODrive.getParameterAsFloat("ibus");

        ss << loxThrottlePos << "," << fuelThrottlePos << ","
           << loxThrottleVel << "," << fuelThrottleVel << ","
           << loxVoltage << "," << fuelVoltage << ","
           << loxCurrent << "," << fuelCurrent;
#endif
        return ss.str();
    }

    /**
     * Returns a string containing the hardware and firmware major and minor versons of the IPA and LOX ODrives and whether
     * they are misconfigured or require a reboot.
     */
    std::string getODriveInfo() {
        std::stringstream ss;
#if (ENABLE_ODRIVE_COMM)
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

        ss << "LOX ODrive Hardware Version: " << loxHWVersionMajor << "." << loxHWVersionMinor
           << " | LOX Firmware Version: " << loxFWVersionMajor << "." << loxFWVersionMinor
           << " | LOX Misconfigured: " << loxMisconfigured
           << " | LOX Reboot Required: " << loxRebootRequired << " ||| ";
        ss << "Fuel ODrive Hardware Version: " << fuelHWVersionMajor << "." << fuelHWVersionMinor
           << " | Fuel Firmware Version: " << fuelFWVersionMajor << "." << fuelFWVersionMinor
           << " | Fuel Misconfigured: " << fuelMisconfigured
           << " | Fuel Reboot Required: " << fuelRebootRequired << "\n";
#endif
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

        if (Router::logenabled) {
            odriveLogFile = createCurveLog();

            if (!odriveLogFile) {
                Router::info("Failed to create odrive log file.");
            }
        }

        bool open = Loader::header.is_open;

        switch (Loader::header.ctype) {
            case curve_type::lerp:
                (open ? followOpenLerpCurve() : followClosedLerpCurve());
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

        if (Router::logenabled) {
            odriveLogFile.flush();
            odriveLogFile.close();
        }

        Router::info("Finished following curve!");
    }
}
