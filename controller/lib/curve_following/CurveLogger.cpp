#include "CurveLogger.h"

#include "pi_controller.h"
#include "CString.h"
#include "Driver.h"
#include "Router.h"
#include "SDCard.h"

namespace CurveLogger {

File odriveLogFile;
CString<400> curveTelemCSV;

#define LOG_HEADER ("time,phase,thrust_cmd,lox_pos_cmd,ipa_pos_cmd,"                                               \
                    "lox_pos,lox_vel,lox_voltage,lox_current,"                                                     \
                    "ipa_pos,ipa_vel,ipa_voltage,ipa_current,"                                                     \
                    "chamber_pressure,"                                                                            \
                    "lox_valve_upstream_pressure,lox_valve_downstream_pressure,lox_venturi_differential_pressure," \
                    "lox_venturi_temperature,lox_valve_temperature,"                                               \
                    "ipa_valve_upstream_pressure,ipa_valve_downstream_pressure,ipa_venturi_differential_pressure," \
                    "chamber_pressure_controller_p_component,chamber_pressure_controller_i_component,"             \
                    "lox_angle_controller_p_component,lox_angle_controller_i_component,"                           \
                    "ipa_angle_controller_p_component,ipa_angle_controller_i_component,"                           \
                    "lox_mdot,ipa_mdot,ol_lox_mdot,ol_ipa_mdot,ol_lox_angle,ol_ipa_angle")

// logs time, phase, thrust, and sensor data in .csv format
int print_counter = 0;
void log_curve_csv(float time, int phase, float thrust, Sensor_Data sd) {
  Controller_State cs = ClosedLoopControllers::getState();
  curveTelemCSV.clear();
  curveTelemCSV << time << "," << phase << "," << thrust << "," << Driver::loxODrive.getLastPosCmd() << "," << Driver::ipaODrive.getLastPosCmd() << ","
                << Driver::loxODrive.getTelemetryCSV() << "," << Driver::ipaODrive.getTelemetryCSV() << ","
                << sd.chamber_pressure << ","
                << sd.ox.valve_upstream_pressure << "," << sd.ox.valve_downstream_pressure << "," << sd.ox.venturi_differential_pressure << ","
                << sd.ox.venturi_temperature << "," << sd.ox.valve_temperature << ","
                << sd.ipa.valve_upstream_pressure << "," << sd.ipa.valve_downstream_pressure << "," << sd.ipa.venturi_differential_pressure << ","
                << cs.chamber_pressure_controller_p_component << "," << cs.chamber_pressure_controller_i_component << ","
                << cs.lox_angle_controller_p_component << "," << cs.lox_angle_controller_i_component << ","
                << cs.ipa_angle_controller_p_component << "," << cs.ipa_angle_controller_i_component << ","
                << vc_state.measured_lox_mdot << "," << vc_state.measured_ipa_mdot << ","
                << vc_state.ol_lox_mdot << "," << vc_state.ol_ipa_mdot << "," << vc_state.ol_lox_angle << "," << vc_state.ol_ipa_angle;

  odriveLogFile.println(curveTelemCSV.str);
  odriveLogFile.flush();

  print_counter++;
  if (print_counter % 10 == 0) {
    curveTelemCSV.clear();
    curveTelemCSV << time << "  " << thrust << "  " << vc_state.measured_lox_mdot << "  " << vc_state.measured_ipa_mdot;
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
