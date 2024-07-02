#include "ODrive.h"

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
