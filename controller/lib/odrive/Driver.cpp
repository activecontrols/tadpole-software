/*
 * Driver.cpp
 *
 *  Created on: 2024-06-10 by Vincent Palmerio
 *  Maintained by Vincent Palmerio and Ishan Goel
 *  Description: This file contains the implementation of the Driver namespace, which provides functions for controlling ODrive motors.
 *               It includes functions for logging telemetry data, creating log files, and following different types of curves.
 *               The curves supported are lerp (linear interpolation), sine, and chirp.
 */

// TODO - log motor temperature

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

namespace Driver {

namespace { // private

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
                << loxODrive.getTelemetryCSV() << "," << " , , , , , "; // TODO RJN - enable telem from 2 valves
  curveTelemCSV.print();
  if (Router::logenabled) {
    odriveLogFile.println(curveTelemCSV.str);
  }
}

/**
 * Creates a log file for the current curve.
 * @return The created log file.
 */
File createCurveLog() {
  // filenames use DOS 8.3 standard
  Router::info_no_newline("Enter log filename (1-8 chars + '.' + 3 chars): ");
  String filename = Router::read(50);
  filename.toUpperCase(); // lower case files have issues on teensy
  File odriveLogFile = SDCard::open(filename.c_str(), FILE_WRITE);

  if (!odriveLogFile) { // Failed to create a log file
    return odriveLogFile;
  }

  odriveLogFile.println(LOG_HEADER);

  return odriveLogFile;
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

/**
 * Follows an open lerp curve by interpolating between LOX and IPA positions.
 */
void followAngleLerpCurve() {
  lerp_point_angle *lac = Loader::lerp_angle_curve;

  elapsedMillis timer = elapsedMillis();
  unsigned long lastlog = timer;

  for (int i = 0; i < Loader::header.num_points - 1; i++) {
    while (timer / 1000.0 < lac[i + 1].time) {
      float seconds = timer / 1000.0;
      float lox_pos = lerp(lac[i].lox_angle, lac[i + 1].lox_angle, lac[i].time, lac[i + 1].time, seconds) / 360;
      float ipa_pos = lerp(lac[i].ipa_angle, lac[i + 1].ipa_angle, lac[i].time, lac[i + 1].time, seconds) / 360;
      loxODrive.setPos(lox_pos);
      // ipaODrive.setPos(ipa_pos);
      if (timer - lastlog > LOG_INTERVAL_MS) {
        logCurveTelemCSV(seconds, i, -1);
        lastlog = timer;
      }

      ZucrowInterface::send_valve_angles_to_zucrow(loxODrive.getPosition(), 0);
      if (Router::check_for_kill()) {
        loxODrive.setParameter("axis0.requested_state", AXIS_STATE_IDLE); // stop movement attempt
        // ipaODrive.setParameter("axis0.requested_state", AXIS_STATE_IDLE); // stop movement attempt
        Router::info("Panic! Loop terminated.");
        break;
      }
      delay(COMMAND_INTERVAL_MS);
    }
  }
}

/**
 * Follows a closed lerp curve by interpolating between thrust values.
 */
void followThrustLerpCurve() {
  lerp_point_thrust *ltc = Loader::lerp_thrust_curve;
  elapsedMillis timer = elapsedMillis();
  unsigned long lastlog = timer;
  Serial.println(timer);

  for (int i = 0; i < Loader::header.num_points - 1; i++) {
    while (timer / 1000.0 < ltc[i + 1].time) {
      float seconds = timer / 1000.0;
      float thrust = lerp(ltc[i].thrust, ltc[i + 1].thrust, ltc[i].time, ltc[i + 1].time, seconds);
      setThrustOpenLoop(thrust);
      if (timer - lastlog >= LOG_INTERVAL_MS) {
        logCurveTelemCSV(seconds, i, thrust);
        lastlog = timer;
      }

      ZucrowInterface::send_valve_angles_to_zucrow(loxODrive.getPosition(), 0);
      if (Router::check_for_kill()) {
        loxODrive.setParameter("axis0.requested_state", AXIS_STATE_IDLE); // stop movement attempt
        // ipaODrive.setParameter("axis0.requested_state", AXIS_STATE_IDLE); // stop movement attempt
        Router::info("Panic! Loop terminated.");
        break;
      }
      delay(COMMAND_INTERVAL_MS);
    }
  }
}

} // namespace

void basic_control_loop(float run_time, float min_p, float max_p) {
  loxODrive.setPos(45 / 360.0);
  delay(1000);
  Router::info("Starting control loop...");
  elapsedMillis timer = elapsedMillis();
  while (timer / 1000.0 < run_time) {
    float pres_dif = loxODrive.pressure_sensor_in->getPressure() - loxODrive.pressure_sensor_out->getPressure();
    Router::info_no_newline("Pres dif: ");
    Router::info_no_newline(pres_dif);
    Router::info_no_newline(" Angle: ");
    Router::info(loxODrive.getLastPosCmd());
    if (pres_dif < min_p) {
      loxODrive.setPos(loxODrive.getLastPosCmd() - 0.0001);
    }

    if (pres_dif > max_p) {
      loxODrive.setPos(loxODrive.getLastPosCmd() + 0.0001);
    }

    ZucrowInterface::send_valve_angles_to_zucrow(loxODrive.getPosition(), 0);
    if (Router::check_for_kill()) {
      loxODrive.setParameter("axis0.requested_state", AXIS_STATE_IDLE); // stop movement attempt
      // ipaODrive.setParameter("axis0.requested_state", AXIS_STATE_IDLE); // stop movement attempt
      Router::info("Panic! Loop terminated.");
      break;
    }
    delay(3);
  }
}

void basic_control_loop_cmd() {
  Router::info_no_newline("Time (seconds): ");
  String respStr = Router::read(INT_BUFFER_SIZE);

  float time = 0.0;
  int result = std::sscanf(respStr.c_str(), "%f", &time);
  if (result != 1) {
    Router::info("Could not convert input to a float, not continuing");
    return;
  }

  Router::info_no_newline("Target Min Pressure (0 - 3.3): ");
  respStr = Router::read(INT_BUFFER_SIZE);

  float min_p = 0.0;
  result = std::sscanf(respStr.c_str(), "%f", &min_p);
  if (result != 1) {
    Router::info("Could not convert input to a float, not continuing");
    return;
  }

  Router::info_no_newline("Target Max Pressure (0 - 3.3): ");
  respStr = Router::read(INT_BUFFER_SIZE);

  float max_p = 0.0;
  result = std::sscanf(respStr.c_str(), "%f", &max_p);
  if (result != 1) {
    Router::info("Could not convert input to a float, not continuing");
    return;
  }
  basic_control_loop(time, min_p, max_p);
}

void begin() {
  Router::add({Driver::followCurve, "follow_curve"});
  Router::add({Driver::printODriveInfo, "get_odrive_info"});
  // Router::add({Driver::setThrustCmd, "set_thrust"});
  Router::add({Driver::setThrustCmd_OPEN_LOOP, "set_thrust_open_loop"});

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

  Router::add({[&]() { loxODrive.kill(); }, "kill"}); // TODO - ipaODrive kill

  Router::add({[&]() { basic_control_loop_cmd(); }, "control_loop_bad"});

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

// /**
//  * Command for the Router lib to change the thrust manually.
//  */
// void setThrustCmd() {
//     Router::info_no_newline("Position?");
//     String thrustString = Router::read(INT_BUFFER_SIZE);
//     Router::info("Response: " + thrustString);

//     float thrust;
//     int result = std::sscanf(thrustString.c_str(), "%f", &thrust);
//     if (result != 1) {
//         Router::info("Could not convert input to a float, not continuing");
//         return;
//     }

//     if (thrust < MIN_THRUST || thrust > MAX_THRUST) {
//         Router::info("Thrust outside defined range in code, not continuing");
//         return;
//     }

//     setThrust(thrust);

//     printBuffer.clear();
//     printBuffer << "Thrust set. LOX pos: " << loxODrive.getLastPosCmd() << " IPA pos: " << ipaODrive.getLastPosCmd();

//     Router::info(printBuffer.str);
// }

/*
 * Sets the thrust
 * Uses a closed loop control to set the angle positions of the odrives
 * using feedback from the pressue sensor. The function will set the odrive positions itself, and
 * modify currentLOXPos and currentIPAPos to what it set the odrive positions to.
 */
// void setThrust(float thrust) {
//     // TODO: closed loop magic

// }

/**
 * Command for the Router lib to change the thrust manually.
 */
void setThrustCmd_OPEN_LOOP() {
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

  setThrustOpenLoop(thrust);

  printBuffer.clear();
  printBuffer << "Thrust set using open loop. LOX pos: " << loxODrive.getLastPosCmd(); // << " IPA pos: " << ipaODrive.getLastPosCmd();

  Router::info(printBuffer.str);
}

/*
 * Sets the thrust using open loop control
 */
void setThrustOpenLoop(float thrust) {
  float angle_ox;
  float angle_fuel;
  open_loop_thrust_control_defaults(thrust, &angle_ox, &angle_fuel);
  loxODrive.setPos(angle_ox / 360);
  // ipaODrive.setPos(angle_fuel / 360);
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
  if (loxODrive.checkConfig()) {
    return;
  }

  if (Router::logenabled) {
    odriveLogFile = createCurveLog();

    if (!odriveLogFile) {
      Router::info_no_newline("Failed to create odrive log file. Send 'y' to continue anyway. ");
      String resp = Router::read(50);
      if (resp != "y") {
        return;
      }
    }
  }

  ZucrowInterface::send_ok_to_zucrow(); // tell zucrow we are ready to go

  while (ZucrowInterface::check_sync_from_zucrow() != ZUCROW_SYNC_RUNNING) {
  }; // wait until zucrow gives us the go

  ZucrowInterface::send_sync_to_zucrow(TEENSY_SYNC_RUNNING);
  Loader::header.is_thrust ? followThrustLerpCurve() : followAngleLerpCurve();
  ZucrowInterface::send_sync_to_zucrow(TEENSY_SYNC_IDLE);

  if (Router::logenabled) {
    odriveLogFile.flush();
    odriveLogFile.close();
  }

  Router::info("Finished following curve!");
}
} // namespace Driver
