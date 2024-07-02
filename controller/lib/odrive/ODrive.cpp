#include <sstream>

#include "ODrive.h"

ODrive::ODrive(Stream &serial) : ODriveUART(serial) {}

/**
 * Checks if communication with ODrive is available by requesting the current state
 * Runs in a while loop until the ODrive is connected 
 */
void ODrive::checkConnection() {
#if (ENABLE_ODRIVE_COMM)
    while (ODriveUART::getState() == AXIS_STATE_UNDEFINED) {
        Router::info("No response from ODrive...");
        delay(100);
    }
    Router::info("Setting odrive to closed loop control...");
    while (ODriveUART::getState() != AXIS_STATE_CLOSED_LOOP_CONTROL) {
        ODriveUART::clearErrors();
        ODriveUART::setState(AXIS_STATE_CLOSED_LOOP_CONTROL);
        delay(10);
    }
#endif
}

/**
 * Set the position for the odrive to control flow through valve.
 * @param pos value to be sent to odrive (valid values are from `MIN_ODRIVE_POS` to `MAX_ODRIVE_POS`).
 */
void ODrive::setPos(float pos) {
#if (ENABLE_ODRIVE_COMM)
    ODriveUART::setPosition(pos);
#endif
    posCmd = pos;
}

/**
 * Checks if there are active errors on the ODrive, if so, store the error in activeError and
 * determine if the ODrive is still armed. If the ODrive is disarmed, store the disarm reason 
 * and return `ODRIVE_ERROR_DISARMED`. If it is still armed, return `ODRIVE_ERROR`.
 */
int ODrive::checkErrors() {
#if (ENABLE_ODRIVE_COMM)
    int activeError = ODriveUART::getParameterAsInt("axis0.active_errors");
    if (activeError != 0) {
        bool isArmed = ODriveUART::getParameterAsInt("axis0.is_armed");
        if (!isArmed) {
            disarmReason = ODriveUART::getParameterAsInt("axis0.disarm_reason");
            return ODRIVE_ERROR_DISARMED;
        }
        return ODRIVE_ERROR;
    }
    return 0;
#endif
}

/**
 * Clears error codes on ODrive and sets disarmReason and activeError variable to 0
 */
void ODrive::clearErrors() {
#if (ENABLE_ODRIVE_COMM)
    ODriveUART::clearErrors();
#endif
    disarmReason = 0;
    activeError = 0;
}

/**
 * Blinks the LED on the ODrive for 5 seconds.
 */
void ODrive::identify() {
    Router::info("Identifying LOX ODrive for 5 seconds...");
#if (ENABLE_ODRIVE_COMM)
    ODriveUART::setParameter("identify", true);
        delay(5000);
    ODriveUART::setParameter("identify", false);
#endif
    Router::info("Done");
}

/**
 * Returns a CSV string containing the ODrive Telemetry information, in the following format:
 * position,velocity,voltage,current
 */
std::string ODrive::getTelemetryCSV() {
    std::stringstream ss;
#if (ENABLE_ODRIVE_COMM)
    float position = ODriveUART::getPosition();

    float velocity = ODriveUART::getVelocity();

    float voltage = ODriveUART::getParameterAsFloat("vbus_voltage");

    float current = ODriveUART::getParameterAsFloat("ibus");

    ss << position << "," << velocity << ","
        << voltage << "," << current;

#endif
    return ss.str();
}

/**
 * Returns a string containing the hardware and firmware major and minor versons of the ODrive
 */
std::string ODrive::getODriveInfo() {
    std::stringstream ss;
#if (ENABLE_ODRIVE_COMM)
    int hwVersionMajor = ODriveUART::getParameterAsInt("hw_version_major");
    int hwVersionMinor = ODriveUART::getParameterAsInt("hw_version_minor");
    int fwVersionMajor = ODriveUART::getParameterAsInt("fw_version_major");
    int fwVersionMinor = ODriveUART::getParameterAsInt("fw_version_minor");

    ss << "ODrive Hardware Version: " << hwVersionMajor << "." << hwVersionMinor
        << " | Firmware Version: " << fwVersionMajor << "." << fwVersionMinor << " |||";
    
#endif
    return ss.str();
}
