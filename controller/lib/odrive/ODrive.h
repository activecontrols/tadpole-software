#ifndef ODRIVE_H
#define ODRIVE_H

#include <Router.h>

#include "ODriveUART.h"

#define ENABLE_ODRIVE_COMM (true)

#define ODRIVE_ERROR (0)
#define ODRIVE_ERROR_DISARMED (-1)

class ODrive : public ODriveUART {

private:

    /*
     * The last position command sent to the odrive
     * Modified only in setPos()
     */
    float posCmd;

    /*
     * The last error the odrive encontered 
     * Modified only by checkErrors()
     * Can be cleared by clearErrors()
     * If there is no last error, then the value will be 0
     */
    int activeError;
    
    /*
     * The error code that made the odrive disarm 
     * Modified only by checkErrors()
     * Can be cleared by clearErrors()
     * If there is no last error, then the value will be 0
     */
    int disarmReason;

public:

    void setPos(float);
    float getLastPosCmd() {return posCmd;}
    void printCmdPos() { Router::info(getLastPosCmd()); }

    int checkErrors();

    void identify();

    int getActiveError() {return activeError;}
    int getDisarmReason() {return disarmReason;}

};

#endif