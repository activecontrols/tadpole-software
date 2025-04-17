#include "CurveFollower.h"

#include "valve_controller.h"
#include "WindowComparator.h"
#include "ZucrowInterface.h"
#include "PressureSensor.h"
#include "pi_controller.h"
#include "Thermocouples.h"
#include "CurveLogger.h"
#include "SDCard.h"
#include "Safety.h"
#include "Driver.h"
#include "Loader.h"
#include "Router.h"

#define LOG_INTERVAL_MS 10
#define COMMAND_INTERVAL_MS 1

namespace CurveFollower {

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

// gets sensor data from PTs and TCs and performs safety checks
Sensor_Data get_sensor_data() {
  Sensor_Data sd;

  sd.ox.tank_pressure = PT::lox_tank.getPressure();
  sd.ox.venturi_throat_pressure = PT::lox_venturi_throat.getPressure();
  sd.ox.venturi_upstream_pressure = PT::lox_venturi_upstream.getPressure();
  sd.ox.valve_temperature = TC::lox_valve_temperature.getTemperature_Kelvin();
  sd.ox.venturi_temperature = TC::lox_venturi_temperature.getTemperature_Kelvin();

  sd.ipa.tank_pressure = PT::ipa_tank.getPressure();
  sd.ipa.venturi_throat_pressure = PT::ipa_venturi_throat.getPressure();
  sd.ipa.venturi_upstream_pressure = PT::ipa_venturi_upstream.getPressure();

  sd.chamber_pressure = PT::chamber.getPressure();

  WindowComparators::lox_tank_pressure.check(sd.ox.tank_pressure);
  WindowComparators::lox_venturi_throat_pressure.check(sd.ox.venturi_throat_pressure);
  WindowComparators::lox_venturi_upstream_pressure.check(sd.ox.venturi_upstream_pressure);
  WindowComparators::lox_valve_temperature.check(sd.ox.valve_temperature);
  WindowComparators::lox_venturi_temperature.check(sd.ox.venturi_temperature);

  WindowComparators::ipa_tank_pressure.check(sd.ipa.tank_pressure);
  WindowComparators::ipa_venturi_throat_pressure.check(sd.ipa.venturi_throat_pressure);
  WindowComparators::ipa_venturi_upstream_pressure.check(sd.ipa.venturi_upstream_pressure);

  WindowComparators::chamber_pressure.check(sd.chamber_pressure);
  return sd;
}

/**
 * Follows an angle curve by interpolating between LOX and IPA positions.
 */
void followAngleLerpCurve() {
  lerp_point_angle *lac = Loader::lerp_angle_curve;
  int kill_reason = DONT_KILL;
  elapsedMillis timer = elapsedMillis();
  unsigned long lastlog = timer;

  WindowComparators::reset();

  for (int i = 0; i < Loader::header.num_points - 1; i++) {
    while (timer / 1000.0 < lac[i + 1].time) {
      float seconds = timer / 1000.0;
      float lox_pos = lerp(lac[i].lox_angle, lac[i + 1].lox_angle, lac[i].time, lac[i + 1].time, seconds) / 360;
      float ipa_pos = lerp(lac[i].ipa_angle, lac[i + 1].ipa_angle, lac[i].time, lac[i + 1].time, seconds) / 360;

      Sensor_Data sd = get_sensor_data();

      Driver::loxODrive.setPos(lox_pos);
      Driver::ipaODrive.setPos(ipa_pos);

      if (timer - lastlog > LOG_INTERVAL_MS) {
        CurveLogger::log_curve_csv(seconds, i, -1, sd);
        lastlog = timer;
      }

      ZucrowInterface::send_valve_angles_to_zucrow(Driver::loxODrive.position, Driver::ipaODrive.position);
      kill_reason = Safety::check_for_kill();
      if (kill_reason != DONT_KILL) {
        Safety::kill_response(kill_reason);
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
 * Follows a thrust curve by interpolating between thrust values.
 */
void followThrustLerpCurve() {
  lerp_point_thrust *ltc = Loader::lerp_thrust_curve;
  int kill_reason = DONT_KILL;
  elapsedMillis timer = elapsedMillis();
  unsigned long lastlog = timer;

  ClosedLoopControllers::reset();
  WindowComparators::reset();

  for (int i = 0; i < Loader::header.num_points - 1; i++) {
    while (timer / 1000.0 < ltc[i + 1].time) {
      float seconds = timer / 1000.0;
      float thrust = lerp(ltc[i].thrust, ltc[i + 1].thrust, ltc[i].time, ltc[i + 1].time, seconds);

      Sensor_Data sd = get_sensor_data();

      float angle_ox;
      float angle_fuel;
      open_loop_thrust_control_defaults(thrust, &angle_ox, &angle_fuel);
      // open_loop_thrust_control(thrust, sd, &angle_ox, &angle_fuel);
      // closed_loop_thrust_control(thrust, sd, &angle_ox, &angle_fuel);
      Driver::loxODrive.setPos(angle_ox / 360);
      Driver::ipaODrive.setPos(angle_fuel / 360);

      if (timer - lastlog >= LOG_INTERVAL_MS) {
        CurveLogger::log_curve_csv(seconds, i, thrust, sd);
        lastlog = timer;
      }

      ZucrowInterface::send_valve_angles_to_zucrow(Driver::loxODrive.position, Driver::ipaODrive.position);
      kill_reason = Safety::check_for_kill();
      if (kill_reason != DONT_KILL) {
        Safety::kill_response(kill_reason);
        break;
      }
      delay(COMMAND_INTERVAL_MS);
    }
    if (kill_reason != DONT_KILL) {
      break;
    }
  }
}

// init CurveFollower and add relevant router cmds
void begin() {
  Router::add({follow_curve_cmd, "follow_curve"});
  Router::add({auto_seq, "auto_seq"});
}

/**
 * Initiates curve following based on the curve header loaded in Loader.cpp.
 */
void follow_curve(const char *log_file_name) {
  // checkConfig() function provides its own error message console logging
  if (Driver::loxODrive.checkConfig() || Driver::ipaODrive.checkConfig()) {
    return;
  }

  CurveLogger::create_curve_log(log_file_name); // lower case files have issues on teensy

  ZucrowInterface::send_ok_to_zucrow(); // tell zucrow we are ready to go

#ifdef ENABLE_ZUCROW_SAFETY
  while (ZucrowInterface::check_sync_from_zucrow() != ZUCROW_SYNC_RUNNING) {
  }; // wait until zucrow gives us the go
#endif

  ZucrowInterface::send_sync_to_zucrow(TEENSY_SYNC_RUNNING);
  Loader::header.is_thrust ? followThrustLerpCurve() : followAngleLerpCurve();
  ZucrowInterface::send_sync_to_zucrow(TEENSY_SYNC_IDLE);

  Router::info("Finished following curve!");
  CurveLogger::close_curve_log();
}

// prompt user for log file name, then follow curve
void follow_curve_cmd() {
  if (!Loader::loaded_curve) {
    Router::info("No curve loaded.");
    return;
  }

  // filenames use DOS 8.3 standard
  Router::info_no_newline("Enter log filename (1-8 chars + '.' + 3 chars): ");
  String filename = Router::read(50);
  follow_curve(filename.toUpperCase().c_str());
}

void auto_seq() { // TODO - home valves?, zero PTs?
  const char *curve_file_name = "AUTOCUR";
  const char *tpl_log_file_name = "AUTOL"; // generates names like AUTOL#.CSV

  if (!Loader::load_curve_sd(curve_file_name)) {
    return;
  };

  String log_file_name = SDCard::get_next_safe_name(tpl_log_file_name);
  Router::info_no_newline("Using log file: ");
  Router::info(log_file_name);
  follow_curve(log_file_name.c_str());
}

} // namespace CurveFollower