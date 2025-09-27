#include "LogController.h"

#include "valve_controller.h"
#include "PressureSensor.h"
#include "Thermocouples.h"
#include "LogWriter.h"
#include "SDCard.h"
#include "Loader.h"
#include "Router.h"

#define LOG_INTERVAL_US 5000
#define COMMAND_INTERVAL_US 1000

namespace LogController {

// gets sensor data from PTs and TCs and performs safety checks
Sensor_Data get_sensor_data() {
  Sensor_Data sd;

  sd.ox.valve_upstream_pressure = PT::lox_valve_upstream.getPressure();
  sd.ox.valve_downstream_pressure = PT::lox_valve_downstream.getPressure();
  sd.ox.venturi_differential_pressure = PT::lox_venturi_differential.getPressure();
  sd.ox.valve_temperature = TC::lox_valve_temperature.getTemperature_Kelvin();
  sd.ox.venturi_temperature = TC::lox_venturi_temperature.getTemperature_Kelvin();

  sd.ipa.valve_upstream_pressure = PT::ipa_valve_upstream.getPressure();
  sd.ipa.valve_downstream_pressure = PT::ipa_valve_downstream.getPressure();
  sd.ipa.venturi_differential_pressure = PT::ipa_venturi_differential.getPressure();

  sd.chamber_pressure = PT::chamber.getPressure();

  return sd;
}

void print_labeled_sensor(const char *msg, float sensor_value, const char *unit) {
  Router::info_no_newline(msg);
  Router::info_no_newline(sensor_value);
  Router::info(unit);
}

void print_all_sensors() {
  Router::info("  Sensor Status ");
  print_labeled_sensor("      PT LOX Valve Upstream: ", PT::lox_valve_upstream.getPressure(), " psi");
  print_labeled_sensor("    PT LOX Valve Downstream: ", PT::lox_valve_downstream.getPressure(), " psi");
  print_labeled_sensor("PT LOX Venturi Differential: ", PT::lox_venturi_differential.getPressure(), " psi");

  print_labeled_sensor("      PT IPA Valve Upstream: ", PT::ipa_valve_upstream.getPressure(), " psi");
  print_labeled_sensor("    PT IPA Valve Downstream: ", PT::ipa_valve_downstream.getPressure(), " psi");
  print_labeled_sensor("PT IPA Venturi Differential: ", PT::ipa_venturi_differential.getPressure(), " psi");

  print_labeled_sensor("                 PT Chamber: ", PT::chamber.getPressure(), " psi");

  print_labeled_sensor("               TC LOX Valve: ", TC::lox_valve_temperature.getTemperature_F(), " F");
  print_labeled_sensor("             TC LOX Venturi: ", TC::lox_venturi_temperature.getTemperature_F(), " F");
  Router::info(" "); // newline
}

void run_log_loop(float log_time_seconds) {
  elapsedMicros timer = elapsedMicros();
  unsigned long lastlog = timer;
  unsigned long lastloop = timer;

  long counter = 0;

  while (timer / 1000000.0 < log_time_seconds) {
    float seconds = timer / 1000000.0;

    Sensor_Data sd = get_sensor_data();
    log_only(sd);

    if (timer - lastlog > LOG_INTERVAL_US) {
      lastlog += LOG_INTERVAL_US;
      LogWriter::log_csv_data(seconds, sd);
    }
    counter++;

    unsigned long target_slp = COMMAND_INTERVAL_US - (timer - lastloop);
    delayMicroseconds(target_slp < COMMAND_INTERVAL_US ? target_slp : 0); // don't delay for too long
    lastloop += COMMAND_INTERVAL_US;
  }
  Router::info_no_newline("Finished ");
  Router::info_no_newline(counter);
  Router::info(" loop iterations.");
}

// add relevant router cmds
void begin() {
  Router::add({print_all_sensors, "print_sensors"});
  Router::add({arm, "arm"});
}

// prompt user for log file name and time, then start logging
void arm() {
  if (!PT::zeroed_since_boot) {
    Router::info("ARMING FAILURE: pt boards have not been zeroed.");
    return;
  }

  // filenames use DOS 8.3 standard
  Router::info_no_newline("Enter log filename (1-8 chars + '.' + 3 chars): ");
  String log_file_name = Router::read(50);
  LogWriter::create_data_log(log_file_name.c_str()); // lower case files have issues on teensy

  float log_time;
  Router::info_no_newline("Enter log time (seconds): ");
  String log_time_string = Router::read(50);
  int result = std::sscanf(log_time_string.c_str(), "%f", &log_time);
  if (result != 1) {
    Router::info("ARMING FAILURE: invalid value entered.");
    return;
  }

  Router::info_no_newline("ARMING COMPLETE. Type `y` and press enter to confirm. ");
  String final_check_str = Router::read(50);
  if (final_check_str != "y") {
    Router::info("ARMING FAILURE: Cancelled by operator.");
    LogWriter::close_data_log();
    return;
  }

  run_log_loop(log_time);

  Router::info("Finished logging data!");
  LogWriter::close_data_log();
}

} // namespace LogController