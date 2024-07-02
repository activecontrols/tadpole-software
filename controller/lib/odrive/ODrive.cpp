
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

