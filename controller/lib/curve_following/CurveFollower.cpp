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

void print_labeled_sensor(const char *msg, float sensor_value, const char *unit) {
  Router::info_no_newline(msg);
  Router::info_no_newline(sensor_value);
  Router::info(unit);
}

void print_all_sensors() {
  Router::info("  Sensor Status ");
  print_labeled_sensor("            PT LOX Tank: ", PT::lox_tank.getPressure(), " psi");
  print_labeled_sensor("PT LOX Venturi Upstream: ", PT::lox_venturi_upstream.getPressure(), " psi");
  print_labeled_sensor("  PT LOX Venturi Throat: ", PT::lox_venturi_throat.getPressure(), " psi");

  print_labeled_sensor("            PT IPA Tank: ", PT::ipa_tank.getPressure(), " psi");
  print_labeled_sensor("PT IPA Venturi Upstream: ", PT::ipa_venturi_upstream.getPressure(), " psi");
  print_labeled_sensor("  PT IPA Venturi Throat: ", PT::ipa_venturi_throat.getPressure(), " psi");

  print_labeled_sensor("             PT Chamber: ", PT::chamber.getPressure(), " psi");

  print_labeled_sensor("           TC LOX Valve: ", TC::lox_valve_temperature.getTemperature_F(), " F");
  print_labeled_sensor("         TC LOX Venturi: ", TC::lox_venturi_temperature.getTemperature_F(), " F");
  Router::info(" "); // newline
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
      log_only(sd);

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
      // log_only(sd);
      open_loop_thrust_control(thrust, sd, &angle_ox, &angle_fuel);
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
  Router::add({print_all_sensors, "print_sensors"});
  Router::add({arm, "arm"});
}

// prompt user for log file name, then follow curve
void arm() {
  if (!Loader::loaded_curve) {
    Router::info("ARMING FAILURE: no curve loaded.");
    return;
  }

  if (!PT::zeroed_since_boot) {
    Router::info("ARMING FAILURE: pt boards have not been zeroed.");
    return;
  }

#ifdef ENABLE_ZUCROW_SAFETY
  if (ZucrowInterface::check_fault_from_zucrow()) {
    Router::info("ARMING FAILURE: Zucrow abort line in abort state.");
    return;
  }

  if (ZucrowInterface::check_sync_from_zucrow() == ZUCROW_SYNC_RUNNING) {
    Router::info("ARMING FAILURE: Zucrow already commanding curve start.");
    return;
  }
#endif

  Router::info("ARMING STATUS: Connecting to odrives.");
  Driver::loxODrive.checkConnection(); // will block until odrive connection is valid
  Driver::ipaODrive.checkConnection();

  Router::info("ARMING STATUS: Preparing to move odrives to start pos.");
  if (Loader::header.is_thrust) {
    float lox_tank_pressure;
    Router::info_no_newline("Enter lox tank pressure (psi): ");
    String lox_tank_string = Router::read(50);
    int result = std::sscanf(lox_tank_string.c_str(), "%f", &lox_tank_pressure);
    if (result != 1) {
      Router::info("ARMING FAILURE: invalid value entered.");
      return;
    }

    float ipa_tank_pressure;
    Router::info_no_newline("Enter ipa tank pressure (psi): ");
    String ipa_tank_string = Router::read(50);
    result = std::sscanf(ipa_tank_string.c_str(), "%f", &ipa_tank_pressure);
    if (result != 1) {
      Router::info("ARMING FAILURE: invalid value entered.");
      return;
    }

    Sensor_Data sd;
    sd.ox.tank_pressure = lox_tank_pressure;
    sd.ipa.tank_pressure = ipa_tank_pressure;
    float lox_pos;
    float ipa_pos;
    open_loop_thrust_control(Loader::lerp_thrust_curve[0].thrust, sd, &lox_pos, &ipa_pos);
    Driver::loxODrive.setPos(lox_pos / 360);
    Driver::ipaODrive.setPos(ipa_pos / 360);
  } else {
    Driver::loxODrive.setPos(Loader::lerp_angle_curve[0].lox_angle / 360);
    Driver::ipaODrive.setPos(Loader::lerp_angle_curve[0].ipa_angle / 360);
  }

  // filenames use DOS 8.3 standard
  Router::info_no_newline("Enter log filename (1-8 chars + '.' + 3 chars): ");
  String log_file_name = Router::read(50);
  CurveLogger::create_curve_log(log_file_name.c_str()); // lower case files have issues on teensy

  Router::info_no_newline("ARMING COMPLETE. Type `y` and press enter to confirm. ");
  String final_check_str = Router::read(50);
  if (final_check_str != "y") {
    Router::info("ARMING FAILURE: Cancelled by operator.");
    CurveLogger::close_curve_log();
    return;
  }

  ZucrowInterface::send_ok_to_zucrow(); // tell zucrow we are ready to go

#ifdef ENABLE_ZUCROW_SAFETY
  elapsedMillis sensor_print_timer = elapsedMillis();
  unsigned long last_print = sensor_print_timer;

  while (ZucrowInterface::check_sync_from_zucrow() != ZUCROW_SYNC_RUNNING) {
    if (COMMS_SERIAL.available() && COMMS_SERIAL.read() == 'k') {
      Router::info("ARMING FAILURE: Aborted by operator.");
      return;
    }
    if (ZucrowInterface::check_fault_from_zucrow()) {
      Router::info("ARMING FAILURE: Zucrow abort line in abort state.");
      return;
    }

    if (sensor_print_timer - last_print >= 1000) {
      Router::info("Waiting for go signal from Zucrow... ");
      print_all_sensors();
      last_print = sensor_print_timer;
    }
  }; // wait until zucrow gives us the go
#endif

  ZucrowInterface::send_sync_to_zucrow(TEENSY_SYNC_RUNNING);
  Loader::header.is_thrust ? followThrustLerpCurve() : followAngleLerpCurve();
  ZucrowInterface::send_sync_to_zucrow(TEENSY_SYNC_IDLE);

  Router::info("Finished following curve!");
  CurveLogger::close_curve_log();
}

} // namespace CurveFollower