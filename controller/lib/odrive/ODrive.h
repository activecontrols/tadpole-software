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
     * Name assigned to the ODrive for more descriptive error codes and
     * console printing. Can be either "LOX" or "IPA" with a null terminator
     */
    char name[4];

    /*
     * The last position command sent to the odrive
     * Modified only in `setPos()`
     */
    float posCmd;

    /*
     * The last error the odrive encontered 
     * Modified only by `checkErrors()`
     * Can be cleared by `ODriveUART::clearErrors()`
     * If there is no last error, then the value will be `0`
     */
    int activeError;
    
    /*
     * The error code that made the odrive disarm 
     * Modified only by `checkErrors()`
     * Can be cleared by `ODriveUART::clearErrors()`
     * If there is no last error, then the value will be `0`
     */
    int disarmReason;

    /*
     * A reference to the serial port object used by the ODrive
     */
    Stream &serial;

    /* 
     * Handler for the thread ( `watchdogThreadFunc` ) that feeds the ODrive watchdog and checks
     * for active errors.
     * Modified by `startWatchdogThread()` and `terminateWatchdogThread()`
     */
    std::thread watchdogThread;

    /* 
     * An atomic used to determine if the watchdog thread has finished execution
     * Based on the solution (third example) here: 
     * https://stackoverflow.com/questions/9094422/how-to-check-if-a-stdthread-is-still-running
     */
    std::atomic<bool> threadExecutionFinished;

public:

    ODrive(Stream &serial, char[3]);

    void checkConnection();
    int checkConfig();

    void setPos(float);
    void setPosConsoleCmd();
    float getLastPosCmd() {return posCmd;}
    void printCmdPos() { Router::info(getLastPosCmd()); }

    int checkErrors();
    void printErrors();
    void startWatchdogThread();
    static int watchdogThreadFunc(Stream&, std::atomic<bool>&);
    void terminateWatchdogThread();
    bool checkThreadExecutionFinished() {return threadExecutionFinished; }
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