#ifndef ODRIVE_H
#define ODRIVE_H

#include "Router.h"
#include "CString.h"
#include "ODriveCAN.h"
#include <FlexCAN_T4.h>
#include "ODriveFlexCAN.hpp"
#define CAN_BAUDRATE 250000

struct ODriveUserData {
  Heartbeat_msg_t last_heartbeat;
  bool received_heartbeat = false;
  Get_Encoder_Estimates_msg_t last_feedback;
  bool received_feedback = false;
};

#define ENABLE_ODRIVE_COMM (false)

#define ODRIVE_NO_ERROR (0)
#define ODRIVE_ACTIVE_ERROR (-1)
#define ODRIVE_ERROR_DISARMED (-2)
#define ODRIVE_MISCONFIGURED (-3)
#define ODRIVE_REBOOT_REQUIRED (-4)
#define ODRIVE_BAD_STATE (-5)
#define ODRIVE_THREAD_ENDED_PREMATURELY (-6)

#define ODRIVE_TELEM_HEADER ("position,velocity,voltage,current,temperature")

#define INT_BUFFER_SIZE (50)
#define MAX_THRUST (600)
#define MIN_THRUST (0)
#define MAX_ODRIVE_POS (90.0 / 360.0)
#define MIN_ODRIVE_POS (0.0 / 360.0)

void onHeartbeatCB(Heartbeat_msg_t &msg, void *user_data);
void onFeedbackCB(Get_Encoder_Estimates_msg_t &msg, void *user_data);
void setup_can(_MB_ptr handler);

class ODrive : public ODriveCAN {

private:
  /*
   * Name assigned to the ODrive for more descriptive error codes and
   * console printing. Can be either "LOX" or "IPA" with a null terminator
   */
  char name[4];

  CString<80> telemetryCSV;
  CString<80> odriveInfo;

  /*
   * The last position command sent to the odrive
   * Modified only in `setPos()`
   */
  float posCmd;

  /*
   * The last error the odrive encontered
   * Modified only by `checkErrors()`
   * Can be cleared by `ODriveCAN::clearErrors()`
   * If there is no last error, then the value will be `0`
   */
  int activeError;

  /*
   * A flag that determines whether the ODrive is armed or not
   * Modified by terminateWatchdogThread() and checkErrors()
   */
  bool isArmed;

  /*
   * A flag that determines whether the ODrive is misconfigured or not
   * Modified by checkConfig()
   */
  bool misconfigured;

  /*
   * A flag that determines whether the ODrive needs to be rebooted
   * Modified by checkConfig()
   */
  bool rebootRequired;

  /*
   * The error code that made the odrive disarm
   * Modified only by `checkErrors()`
   * Can be cleared by `ODriveCAN::clearErrors()`
   * If there is no last error, then the value will be `0`
   */
  int disarmReason;

  /*
   * The major and minor version of the hardware and firmware of the ODrive
   * Modified by `getODriveInfo()`
   */
  int hwVersionMajor;
  int hwVersionMinor;
  int fwVersionMajor;
  int fwVersionMinor;

public:
  ODrive(uint32_t can_id, char[4]);

  /*
   * The last known position, velocity, voltage, and current of the ODrive
   * Modified by `getTelemetryCSV()`
   */
  float position;
  float velocity;
  float voltage;
  float current;
  float temperature;

  // stores data sent back from the odrive - modified by onHeartbeat() and onFeedback()
  ODriveUserData odrive_status;

  void checkConnection();
  int checkConfig();

  void setPos(float);
  void setPosConsoleCmd();
  float getLastPosCmd() { return posCmd; }
  void printCmdPos() { Router::info(getLastPosCmd()); }

  int checkErrors();
  void printErrors();
  void clear();

  void identify();

  int getActiveError() { return activeError; }
  int getDisarmReason() { return disarmReason; }

  char *getTelemetryCSV();
  void printTelemetryCSV() {
    Router::info(ODRIVE_TELEM_HEADER);
    getTelemetryCSV();
    telemetryCSV.print();
  }
  char *getODriveInfo();
  void printODriveInfo() { odriveInfo.print(); }
  void hardStopHoming();
  void indexHoming();
  void kill();
};

#endif