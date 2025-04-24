/*
 * Driver.cpp
 *
 *  Created on: 2024-06-10 by Vincent Palmerio
 *  Maintained by Vincent Palmerio and Ishan Goel
 *  Description: This file contains the implementation of the Driver namespace, which provides functions for controlling ODrive motors.
 *               It includes functions for logging telemetry data, creating log files, and following different types of curves.
 *               The curves supported are lerp (linear interpolation), sine, and chirp.
 */

#include "Driver.h"

namespace Driver {

#define LOX_ODRIVE_CAN_ID 1
#define IPA_ODRIVE_CAN_ID 2

char loxName[4] = "LOX";
char ipaName[4] = "IPA";
ODrive loxODrive(LOX_ODRIVE_CAN_ID, loxName);
ODrive ipaODrive(IPA_ODRIVE_CAN_ID, ipaName);

void onCanMessage(const CanMsg &msg) {
  onReceive(msg, loxODrive);
  onReceive(msg, ipaODrive);
}

void printODriveInfo() {
  Router::info("LOX ODrive: ");
  Router::info(loxODrive.getODriveInfo());
  Router::info("IPA ODrive: ");
  Router::info(ipaODrive.getODriveInfo());
}

void begin() {
  setup_can(onCanMessage);
  Router::add({printODriveInfo, "get_odrive_info"});

  /*
   * This syntax is slightly tricky. The add function only takes one argument: a func struct
   * The func struct has two fields, a function pointer, and a string.
   * Ie: `[&]() {loxODrive.clearErrors(); }` is the "function pointer" and "clear_odrive_errors"
   * is the string. Both these fields are wrapped in curly brackes to create a func struct
   * `{   [&]() {loxODrive.clearErrors(); }, "clear_odrive_errors"   }`
   *
   * For the function pointer itself, we pass in what is called a lambda.
   * (Read up here: https://stackoverflow.com/questions/7627098/what-is-a-lambda-expression-and-when-should-i-use-one)
   *
   * The [&] captures all local variables by reference, so loxODrive and ipaODrive can be called
   * in the lambda.
   *
   * () is the return type, which is void
   *
   * {} inside the brackets, and code can be written, including loxODrive.clearErrors();
   *
   * Together, that makes `[&]() { loxODrive.clearErrors(); }`
   */

  Router::add({[&]() { loxODrive.clear(); }, "clear_lox_odrive_errors"});
  Router::add({[&]() { ipaODrive.clear(); }, "clear_ipa_odrive_errors"});

  Router::add({[&]() { loxODrive.setPosConsoleCmd(); }, "set_lox_odrive_pos"});
  Router::add({[&]() { ipaODrive.setPosConsoleCmd(); }, "set_ipa_odrive_pos"});

  Router::add({[&]() { loxODrive.printCmdPos(); }, "get_lox_cmd_pos"});
  Router::add({[&]() { ipaODrive.printCmdPos(); }, "get_ipa_cmd_pos"});

  Router::add({[&]() { loxODrive.identify(); }, "identify_lox_odrive"});
  Router::add({[&]() { ipaODrive.identify(); }, "identify_ipa_odrive"});

  Router::add({[&]() { loxODrive.printTelemetryCSV(); }, "get_lox_odrive_telem"});
  Router::add({[&]() { ipaODrive.printTelemetryCSV(); }, "get_ipa_odrive_telem"});

  Router::add({[&]() { loxODrive.hardStopHoming(); }, "lox_hard_stop_home"});
  Router::add({[&]() { ipaODrive.hardStopHoming(); }, "ipa_hard_stop_home"});

  Router::add({[&]() { loxODrive.kill(); ipaODrive.kill(); }, "kill"});

#if (ENABLE_ODRIVE_COMM)
  Router::info("Connecting to lox odrive...");
  loxODrive.checkConnection();

  Router::info("Connecting to ipa odrive...");
  ipaODrive.checkConnection();

  printODriveInfo();
#endif
}

} // namespace Driver
