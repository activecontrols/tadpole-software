#include "ODrive.h"

/**
 * Set the position for the odrive to control flow through valve.
 * @param pos value to be sent to odrive (valid values are from MIN_ODRIVE_POS to MAX_ODRIVE_POS).
 */
void ODrive::setPos(float pos) {
#if (ENABLE_ODRIVE_COMM)
    ODriveUART::setPosition(pos);
#endif
    posCmd = pos;
}

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