#include "ODrive.h"
#include <string>

#include <FlexCAN_T4.h>
#include "ODriveFlexCAN.hpp"
struct ODriveStatus; // hack to prevent teensy compile error

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can_intf;
#define ODRV0_NODE_ID 0
#define CAN_BAUDRATE 1000000

void onCanMessage(const CanMsg &msg) {
}

ODrive::ODrive(char name[4]) : ODriveCAN(wrap_can_intf(can_intf), ODRV0_NODE_ID) {
  can_intf.begin();
  can_intf.setBaudRate(CAN_BAUDRATE);
  can_intf.setMaxMB(16);
  can_intf.enableFIFO();
  can_intf.enableFIFOInterrupt();
  // can_intf.onReceive(); // TODO - RJN TODO

  // crappy way of doing this, but it is three characters, so it is fine
  this->name[0] = name[0];
  this->name[1] = name[1];
  this->name[2] = name[2];
  this->name[3] = '\0';
}

/**
 * Checks if communication with ODrive is available by requesting the current state
 * Runs in a while loop until the ODrive is connected
 */
// void ODrive::checkConnection() {
// #if (ENABLE_ODRIVE_COMM)
//   while (ODriveCAN::getState() == AXIS_STATE_UNDEFINED) {
//     Router::info("No response from ODrive...");
//     delay(100);
//   }
//   Router::info("Setting odrive to closed loop control...");
//   while (ODriveCAN::getState() != AXIS_STATE_CLOSED_LOOP_CONTROL) {
//     ODriveCAN::clearErrors();
//     ODriveCAN::setState(AXIS_STATE_CLOSED_LOOP_CONTROL);
//     delay(10);
//   }
// #endif
// }

// /**
//  * Checks the `misconfigured` and `reboot_required` flags (from the ODrive API), and if either
//  * are true, return an error
//  */
// int ODrive::checkConfig() {
// #if (ENABLE_ODRIVE_COMM)
//   misconfigured = ODriveCAN::getParameterAsInt("misconfigured");
//   rebootRequired = ODriveCAN::getParameterAsInt("reboot_required");

//   if (misconfigured) {
//     Router::info(
//         "ERROR: " + std::string(name, 3) +
//         " ODRIVE IS MISCONFIGURED, WILL NOT ALLOW CURVE FOLLOWING");
//     return ODRIVE_MISCONFIGURED;
//   }

//   if (rebootRequired) {
//     Router::info(
//         "ERROR: " + std::string(name, 3) +
//         " ODRIVE NEEDS TO REBOOT, WILL NOT ALLOW CURVE FOLLOWING");
//     return ODRIVE_REBOOT_REQUIRED;
//   }
// #endif
//   return ODRIVE_NO_ERROR;
// }

/**
 * Set the position for the odrive to control flow through valve.
 * @param pos value to be sent to odrive (valid values are from `MIN_ODRIVE_POS` to `MAX_ODRIVE_POS`).
 */
void ODrive::setPos(float pos) {
#if (ENABLE_ODRIVE_COMM)
  ODriveCAN::setPosition(pos);
#endif
  posCmd = pos;
}

/**
 * Checks if there are active errors on the ODrive, if so, store the error in activeError and
 * determine if the ODrive is still armed. If the ODrive is disarmed, store the disarm reason
 * and return `ODRIVE_ERROR_DISARMED`. If it is still armed, return `ODRIVE_ERROR`.
 */
// int ODrive::checkErrors() {
// #if (ENABLE_ODRIVE_COMM)
//   activeError = ODriveCAN::getParameterAsInt("axis0.active_errors");
//   if (activeError != 0) {
//     isArmed = ODriveCAN::getParameterAsInt("axis0.is_armed");
//     if (!isArmed) {
//       disarmReason = ODriveCAN::getParameterAsInt("axis0.disarm_reason");
//       return ODRIVE_ERROR_DISARMED;
//     }
//     return ODRIVE_ACTIVE_ERROR;
//   }
// #endif
//   return ODRIVE_NO_ERROR;
// }

void ODrive::printErrors() {
#if (ENABLE_ODRIVE_COMM)
  checkErrors();
  Router::info(std::string(name, 3) + " ODRIVE active error: " + std::to_string(activeError));
  Router::info(std::string(name, 3) + " ODRIVE disarm reason: " + std::to_string(disarmReason));
#endif
}

/**
 * Clears error codes on ODrive and sets disarmReason and activeError variable to 0
 */
void ODrive::clear() {
#if (ENABLE_ODRIVE_COMM)
  ODriveCAN::clearErrors();
#endif
  disarmReason = 0;
  activeError = 0;
}

/**
 * Blinks the LED on the ODrive for 5 seconds.
 */
// void ODrive::identify() {
//   Router::info("Identifying LOX ODrive for 5 seconds...");
// #if (ENABLE_ODRIVE_COMM)
//   ODriveCAN::setParameter("identify", true);
//   delay(5000);
//   ODriveCAN::setParameter("identify", false);
// #endif
//   Router::info("Done");
// }

/**
 * Returns a CSV string containing the ODrive Telemetry information, in the following format:
 * position,velocity,voltage,current
 */
char *ODrive::getTelemetryCSV() {
  telemetryCSV.clear();

#if (ENABLE_ODRIVE_COMM)
  Get_Encoder_Estimates_msg_t enc_msg;
  ODriveCAN::getFeedback(enc_msg);
  position = enc_msg.Pos_Estimate;
  velocity = enc_msg.Vel_Estimate;

  Get_Bus_Voltage_Current_msg_t vc_msg;
  ODriveCAN::getBusVI(vc_msg);
  voltage = vc_msg.Bus_Voltage;
  current = vc_msg.Bus_Current;

  telemetryCSV << position << "," << velocity << ","
               << voltage << "," << current;
#endif

  return telemetryCSV.str;
}

/**
 * Returns a string containing the hardware and firmware major and minor versons of the ODrive
 */
char *ODrive::getODriveInfo() {
  odriveInfo.clear();

#if (ENABLE_ODRIVE_COMM)
  Get_Version_msg_t msg;
  ODriveCAN::getVersion(msg);
  hwVersionMajor = msg.Hw_Version_Major;
  hwVersionMinor = msg.Hw_Version_Minor;
  fwVersionMajor = msg.Fw_Version_Major;
  fwVersionMinor = msg.Fw_Version_Minor;

  odriveInfo << "ODrive Hardware Version: " << hwVersionMajor << "." << hwVersionMinor
             << " | Firmware Version: " << fwVersionMajor << "." << fwVersionMinor << " |||";
#endif

  return odriveInfo.str;
}

/**
 * Command for the Router lib to change the position of the ODrive manually.
 */
void ODrive::setPosConsoleCmd() {

  Router::info("Position?");
  String posString = Router::read(INT_BUFFER_SIZE);
  Router::info("Response: " + posString);

  float pos = 0.0;
  int result = std::sscanf(posString.c_str(), "%f", &pos);
  if (result != 1) {
    Router::info("Could not convert input to a float, not continuing");
    return;
  }

  if (pos < MIN_ODRIVE_POS || pos > MAX_ODRIVE_POS) {
    Router::info("Position outside defined range in code, not continuing");
    return;
  }

  setPos(pos);
  Router::info("Position set");
}
