#ifndef ODRIVE_H
#define ODRIVE_H

#include "ODriveUART.h"

#define ENABLE_ODRIVE_COMM true

class ODrive : public ODriveUART {

private:

    /*
     * The last position command sent to the odrive
     * Modified only in setPos()
     */
    float posCmd;
    
public:

    void setPos(float);
};

#endif