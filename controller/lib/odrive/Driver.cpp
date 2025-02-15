/*
 * Driver.cpp
 *
 *  Created on: 2024-06-10 by Vincent Palmerio
 *  Maintained by Vincent Palmerio and Ishan Goel
 *  Description: This file contains the implementation of the Driver namespace, which provides functions for controlling ODrive motors.
 *               It includes functions for logging telemetry data, creating log files, and following different types of curves.
 *               The curves supported are lerp (linear interpolation), sine, and chirp.
 */

#include <Arduino.h>
#include <TeensyThreads.h>

#include "zucrow_interface.hpp"
#include "SDCard.h"
#include "Driver.h"
#include "ODrive.h"
#include "CString.h"

ThreadWrap(Serial1, SerialXtra1);
#define Serial1 ThreadClone(SerialXtra1)

ThreadWrap(Serial2, SerialXtra2);
#define Serial2 ThreadClone(SerialXtra2)

#define LOX_ODRIVE_SERIAL (Serial1)
#define IPA_ODRIVE_SERIAL (Serial2)

#define LOG_INTERVAL_MS 10
#define COMMAND_INTERVAL_MS 1

#define CHECK_SERIAL_KILL // should check for 'k' on serial monitor to kill
// #define ENABLE_ZUCROW_SAFETY // checks for zucrow ok before starting
// #define ENABLE_ODRIVE_SAFETY_CHECKS // check if odrive disconnects or falls behind

namespace Driver {

char loxName[4] = "LOX";
// char ipaName[4] = "IPA";
ODrive loxODrive(LOX_ODRIVE_SERIAL, loxName, &Pressure::lox_pressure_in, &Pressure::lox_pressure_out);
// ODrive loxODrive(IPA_ODRIVE_SERIAL, ipaName, &Pressure::ipa_pressure_in, &Pressure::ipa_pressure_out);

File odriveLogFile;

CString<200> curveTelemCSV;
CString<100> printBuffer;

/**
 * Logs the telemetry data for a curve in CSV format.
 * @param time The elapsed time in seconds.
 * @param phase The current phase of the curve.
 * @param thrust The thrust value (for closed lerp curves) or -1 (for other curve types).
 */
void logCurveTelemCSV(float time, int phase, float thrust) {
  curveTelemCSV.clear();
  curveTelemCSV << time << "," << phase << "," << thrust << "," << loxODrive.getLastPosCmd() << "," << "ipa_pos_cmd" << ","
                << loxODrive.getTelemetryCSV() << "," << " , , , , , "; // TODO RJN odrive - enable telem from 2 valves
  curveTelemCSV.print();
  if (Router::logenabled) {
    odriveLogFile.println(curveTelemCSV.str);
  }
}

/**
 * Creates a log file for the current curve.
 * @return The created log file.
 */
void createCurveLog(const char *filename) {
  odriveLogFile = SDCard::open(filename, FILE_WRITE);
  odriveLogFile.println(LOG_HEADER);
}

/**
 * Performs linear interpolation between two values.
 * @param a The starting value.
 * @param b The ending value.
 * @param t0 The starting time.
 * @param t1 The ending time.
 * @param t The current time.
 * @return The interpolated value at the current time.
 */
float lerp(float a, float b, float t0, float t1, float t) {
  if (t <= t0)
    return a;
  if (t >= t1)
    return b;
  if (t0 == t1)
    return b; // immediately get to b
  return a + (b - a) * ((t - t0) / (t1 - t0));
}

void kill_response(int kill_reason) {
  loxODrive.setParameter("axis0.requested_state", AXIS_STATE_IDLE); // stop movement attempt
  // ipaODrive.setParameter("axis0.requested_state", AXIS_STATE_IDLE); // stop movement attempt
  ZucrowInterface::send_fault_to_zucrow();
  Router::info("Panic! Loop terminated.");
  Router::info_no_newline("Kill code: ");
  Router::info(kill_reason);
}

int check_for_kill() {
#ifdef ENABLE_ZUCROW_SAFETY
  if (ZucrowInterface::check_fault_from_zucrow()) {
    return KILLED_BY_ZUCROW;
  }
#endif

#ifdef CHECK_SERIAL_KILL
  if (COMMS_SERIAL.available() && COMMS_SERIAL.read() == 'k') {
    return KILLED_BY_SERIAL;
  }
#endif

#ifdef ENABLE_ODRIVE_SAFETY_CHECKS
  if (abs(loxODrive.position - loxODrive.getLastPosCmd()) > ANGLE_OOR_THRESH) { // TODO RJN odrive - check both motors
    return KILLED_BY_ANGLE_OOR;
  }

  if (loxODrive.getState() != AXIS_STATE_CLOSED_LOOP_CONTROL) {
    return KILLED_BY_ODRIVE_FAULT;
  }
#endif

  return DONT_KILL;
}

/**
 * Follows an open lerp curve by interpolating between LOX and IPA positions.
 */
void followAngleLerpCurve() {
  lerp_point_angle *lac = Loader::lerp_angle_curve;
  int kill_reason = DONT_KILL;
  elapsedMillis timer = elapsedMillis();
  unsigned long lastlog = timer;

  for (int i = 0; i < Loader::header.num_points - 1; i++) {
    while (timer / 1000.0 < lac[i + 1].time) {
      float seconds = timer / 1000.0;
      float lox_pos = lerp(lac[i].lox_angle, lac[i + 1].lox_angle, lac[i].time, lac[i + 1].time, seconds) / 360;
      // float ipa_pos = lerp(lac[i].ipa_angle, lac[i + 1].ipa_angle, lac[i].time, lac[i + 1].time, seconds) / 360;
      loxODrive.setPos(lox_pos);
      // ipaODrive.setPos(ipa_pos);
      if (timer - lastlog > LOG_INTERVAL_MS) {
        logCurveTelemCSV(seconds, i, -1);
        lastlog = timer;
      }

      ZucrowInterface::send_valve_angles_to_zucrow(lox_pos, 0);
      kill_reason = check_for_kill();
      if (kill_reason != DONT_KILL) {
        kill_response(kill_reason);
        break;
      }
      delay(COMMAND_INTERVAL_MS);
    }
    if (kill_reason != DONT_KILL) {
      break;
    }
  }
}

/**
 * Follows a closed lerp curve by interpolating between thrust values.
 */
void followThrustLerpCurve() {
  lerp_point_thrust *ltc = Loader::lerp_thrust_curve;
  int kill_reason = DONT_KILL;
  elapsedMillis timer = elapsedMillis();
  unsigned long lastlog = timer;

  for (int i = 0; i < Loader::header.num_points - 1; i++) {
    while (timer / 1000.0 < ltc[i + 1].time) {
      float seconds = timer / 1000.0;
      float thrust = lerp(ltc[i].thrust, ltc[i + 1].thrust, ltc[i].time, ltc[i + 1].time, seconds);

      float angle_ox;
      float angle_fuel;
      open_loop_thrust_control_defaults(thrust, &angle_ox, &angle_fuel); // TODO CL - make this closed loop
      loxODrive.setPos(angle_ox / 360);
      // ipaODrive.setPos(angle_fuel / 360);

      if (timer - lastlog >= LOG_INTERVAL_MS) {
        logCurveTelemCSV(seconds, i, thrust);
        lastlog = timer;
      }

      ZucrowInterface::send_valve_angles_to_zucrow(loxODrive.position, 0);
      kill_reason = check_for_kill();
      if (kill_reason != DONT_KILL) {
        kill_response(kill_reason);
        break;
      }
      delay(COMMAND_INTERVAL_MS);
    }
    if (kill_reason != DONT_KILL) {
      break;
    }
  }
}

void begin() {
#ifndef ENABLE_ZUCROW_SAFETY
  Router::info("WARNING! Running without zucrow checks.");
#endif
#ifndef ENABLE_ODRIVE_SAFETY_CHECKS
  Router::info("WARNING! Running without odrive safety.");
#endif

  Router::add({Driver::followCurveCmd, "follow_curve"});
  Router::add({Driver::printODriveInfo, "get_odrive_info"});
  Router::add({Driver::setThrustCmd, "set_thrust"});

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
  // Router::add({[&]() {ipaODrive.clear(); }, "clear_ipa_odrive_errors"});

  Router::add({[&]() { loxODrive.setPosConsoleCmd(); }, "set_lox_odrive_pos"});
  // Router::add({[&]() {ipaODrive.setPosConsoleCmd(); }, "set_ipa_odrive_pos"});

  Router::add({[&]() { loxODrive.printCmdPos(); }, "get_lox_cmd_pos"});
  // Router::add({[&]() {ipaODrive.printCmdPos(); }, "get_ipa_cmd_pos"});

  Router::add({[&]() { loxODrive.identify(); }, "identify_lox_odrive"});
  // Router::add({[&]() {ipaODrive.identify(); }, "identify_ipa_odrive"});

  Router::add({[&]() { loxODrive.printTelemetryCSV(); }, "get_lox_odrive_telem"});
  // Router::add({[&]() {ipaODrive.printTelemetryCSV(); }, "get_ipa_odrive_telem"});

  Router::add({[&]() { loxODrive.hardStopHoming(); }, "lox_hard_stop_home"});
  // Router::add({[&]() { ipaODrive.hardStopHoming(); }, "ipa_hard_stop_home"});

  Router::add({[&]() { loxODrive.indexHoming(); }, "lox_idx_homing"});
  // Router::add({[&]() { ipaODrive.indexHoming(); }, "ipa_idx_homing"});

  Router::add({[&]() { loxODrive.printPressure(); }, "lox_print_pressure"});
  // Router::add({[&]() { ipaODrive.printPressure(); }, "lox_print_pressure"});

  Router::add({[&]() { loxODrive.kill(); }, "kill"}); // TODO RJN odrive - ipaODrive kill

#if (ENABLE_ODRIVE_COMM)
  LOX_ODRIVE_SERIAL.begin(LOX_ODRIVE_SERIAL_RATE);
  IPA_ODRIVE_SERIAL.begin(IPA_ODRIVE_SERIAL_RATE);

  Router::info("Connecting to lox odrive...");
  loxODrive.checkConnection();
  loxODrive.checkConfig();

  // Router::info("Connecting to ipa odrive...");
  // ipaODrive.checkConnection();
  // ipaODrive.checkConfig();

  printODriveInfo();
#endif
}

void printODriveInfo() {
  Router::info("LOX ODrive: ");
  Router::info(loxODrive.getODriveInfo());
  // Router::info("IPA ODrive: ");
  // Router::info(ipaODrive.getODriveInfo());
}

/**
 * Command for the Router lib to change the thrust manually.
 */
void setThrustCmd() {
  Router::info_no_newline("Thrust?");
  String thrustString = Router::read(INT_BUFFER_SIZE);
  Router::info("Response: " + thrustString);

  float thrust;
  int result = std::sscanf(thrustString.c_str(), "%f", &thrust);
  if (result != 1) {
    Router::info("Could not convert input to a float, not continuing");
    return;
  }

  if (thrust < MIN_THRUST || thrust > MAX_THRUST) {
    Router::info("Thrust outside defined range in code, not continuing");
    return;
  }

  float angle_ox;
  float angle_fuel;
  open_loop_thrust_control_defaults(thrust, &angle_ox, &angle_fuel);
  loxODrive.setPos(angle_ox / 360);
  // ipaODrive.setPos(angle_fuel / 360);

  printBuffer.clear();
  printBuffer << "Thrust set using open loop. LOX pos: " << loxODrive.getLastPosCmd(); // << " IPA pos: " << ipaODrive.getLastPosCmd();

  Router::info(printBuffer.str);
}

/**
 * Initiates curve following based on the curve header loaded in Loader.cpp.
 */
void followCurve() {
  if (!Loader::loaded_curve) {
    Router::info("No curve loaded.");
    return;
  }

  // checkConfig() function provides its own error message console logging
  if (loxODrive.checkConfig()) { // TODO - both valves
    return;
  }

  ZucrowInterface::send_ok_to_zucrow(); // tell zucrow we are ready to go

#ifdef ENABLE_ZUCROW_SAFETY
  while (ZucrowInterface::check_sync_from_zucrow() != ZUCROW_SYNC_RUNNING) {
  }; // wait until zucrow gives us the go
#endif

  ZucrowInterface::send_sync_to_zucrow(TEENSY_SYNC_RUNNING);
  Loader::header.is_thrust ? followThrustLerpCurve() : followAngleLerpCurve();
  ZucrowInterface::send_sync_to_zucrow(TEENSY_SYNC_IDLE);

  if (Router::logenabled) {
    odriveLogFile.flush();
    odriveLogFile.close();
  }

  Router::info("Finished following curve!");
}

void followCurveCmd() {
  if (Router::logenabled) {
    // filenames use DOS 8.3 standard
    Router::info_no_newline("Enter log filename (1-8 chars + '.' + 3 chars): ");
    String filename = Router::read(50);
    createCurveLog(filename.toUpperCase().c_str()); // lower case files have issues on teensy

    if (!odriveLogFile) {
      Router::info_no_newline("Failed to create odrive log file. Send 'y' to continue anyway. ");
      String resp = Router::read(50);
      if (resp != "y") {
        return;
      }
    }
  }
  followCurve();
}
} // namespace Driver
