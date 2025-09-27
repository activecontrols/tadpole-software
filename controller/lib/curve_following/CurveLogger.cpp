#include "CurveLogger.h"

#include "CString.h"
#include "Router.h"
#include "SDCard.h"

namespace CurveLogger {

File odriveLogFile;
CString<400> curveTelemCSV;

#define LOG_HEADER ("time,chamber_pressure,"                                                                       \
                    "lox_valve_upstream_pressure,lox_valve_downstream_pressure,lox_venturi_differential_pressure," \
                    "lox_venturi_temperature,lox_valve_temperature,"                                               \
                    "ipa_valve_upstream_pressure,ipa_valve_downstream_pressure,ipa_venturi_differential_pressure," \
                    "lox_mdot,ipa_mdot")

// logs time, phase, thrust, and sensor data in .csv format
int print_counter = 0;
void log_curve_csv(float time, Sensor_Data sd) {
  curveTelemCSV.clear();
  curveTelemCSV << time << "," << sd.chamber_pressure << ","
                << sd.ox.valve_upstream_pressure << "," << sd.ox.valve_downstream_pressure << "," << sd.ox.venturi_differential_pressure << ","
                << sd.ox.venturi_temperature << "," << sd.ox.valve_temperature << ","
                << sd.ipa.valve_upstream_pressure << "," << sd.ipa.valve_downstream_pressure << "," << sd.ipa.venturi_differential_pressure << ","
                << vc_state.measured_lox_mdot << "," << vc_state.measured_ipa_mdot;

  odriveLogFile.println(curveTelemCSV.str);
  odriveLogFile.flush();

  print_counter++;
  if (print_counter % 10 == 0) {
    curveTelemCSV.clear();
    curveTelemCSV << time << "  " << "  " << vc_state.measured_lox_mdot << "  " << vc_state.measured_ipa_mdot;
    curveTelemCSV.print();
  }
}

// creates a log file for the current curve and prints csv header
void create_curve_log(const char *filename) {
  odriveLogFile = SDCard::open(filename, FILE_WRITE);
  odriveLogFile.println(LOG_HEADER);
}

// close and flush the log file
void close_curve_log() {
  odriveLogFile.flush();
  odriveLogFile.close();
}

} // namespace CurveLogger
