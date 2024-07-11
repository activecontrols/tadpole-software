#ifndef ODRIVE_H
#define ODRIVE_H

#include <Router.h>
#include <string.h>
#include <atomic>
#include <TeensyThreads.h>

#include "ODriveUART.h"

#define ENABLE_ODRIVE_COMM (true)

#define ODRIVE_NO_ERROR (0)
#define ODRIVE_ACTIVE_ERROR (-1)
#define ODRIVE_ERROR_DISARMED (-2)
#define ODRIVE_MISCONFIGURED (-3)
#define ODRIVE_REBOOT_REQUIRED (-4)
#define ODRIVE_BAD_STATE (-5)
#define ODRIVE_THREAD_ENDED_PREMATURELY (-6)

#define ODRIVE_TELEM_HEADER ("position,velocity,voltage,current")

#define INT_BUFFER_SIZE (50)
#define MAX_THRUST (100)
#define MIN_TRHUST (0)
#define MAX_ODRIVE_POS (1)
#define MIN_ODRIVE_POS (-1)

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
     * Can be cleared by ODriveUART::clearErrors()
     * If there is no last error, then the value will be 0
     */
    int activeError;
    
    /*
     * The error code that made the odrive disarm 
     * Modified only by checkErrors()
     * Can be cleared by ODriveUART::clearErrors()
     * If there is no last error, then the value will be 0
     */
    int disarmReason;

    Stream &serial;
    std::thread watchdogThread;

public:

    ODrive(Stream &serial);

    void checkConnection();
    int checkConfig();

    void setPos(float);
    void setPosConsoleCmd();
    float getLastPosCmd() {return posCmd;}
    void printCmdPos() { Router::info(getLastPosCmd()); }

    int checkErrors();
    void startWatchdogThread();
    static int watchdogThreadFunc(Stream &serial);
    int terminateWatchdogThread();
    static String readLine(Stream&, unsigned long timeout_ms = 10);
    void clear();

    void identify();

    int getActiveError() {return activeError;}
    int getDisarmReason() {return disarmReason;}

    std::string getTelemetryCSV();
    void printTelemetryCSV() {
        Router::info(ODRIVE_TELEM_HEADER);
        Router::info(getTelemetryCSV());
    }
    std::string getODriveInfo();
    void printODriveInfo() { Router::info(getODriveInfo()); }

};

#endif