#include "CurveLogger.h"

#include "pi_controller.h"
#include "CString.h"
#include "Driver.h"
#include "Router.h"
#include "SDCard.h"

namespace CurveLogger {

File odriveLogFile;
CString<400> curveTelemCSV;

#define LOG_HEADER ("time,phase,thrust_cmd,lox_pos_cmd,ipa_pos_cmd,"                                   \
                    "lox_pos,lox_vel,lox_voltage,lox_current,lox_temperature,"                         \
                    "ipa_pos,ipa_vel,ipa_voltage,ipa_current,ipa_temperature,"                         \
                    "lox_tank_pressure,lox_venturi_upstream_pressure,lox_venturi_throat_pressure,"     \
                    "lox_venturi_temperature,lox_valve_temperature,"                                   \
                    "ipa_tank_pressure,ipa_venturi_upstream_pressure,ipa_venturi_throat_pressure,"     \
                    "chamber_pressure_controller_p_component,chamber_pressure_controller_i_component," \
                    "lox_angle_controller_p_component,lox_angle_controller_i_component,"               \
                    "ipa_angle_controller_p_component,ipa_angle_controller_i_component,"               \
                    "lox_mdot,ipa_mdot,ol_lox_mdot,ol_ipa_mdot,ol_lox_angle,ol_ipa_angle")

// logs time, phase, thrust, and sensor data in .csv format
void log_curve_csv(float time, int phase, float thrust, Sensor_Data sd) {
  Controller_State cs = ClosedLoopControllers::getState();
  curveTelemCSV.clear();
  curveTelemCSV << time << "," << phase << "," << thrust << "," << Driver::loxODrive.getLastPosCmd() << "," << Driver::ipaODrive.getLastPosCmd() << ","
                << Driver::loxODrive.getTelemetryCSV() << "," << Driver::ipaODrive.getTelemetryCSV() << ","
                << sd.ox.tank_pressure << "," << sd.ox.venturi_upstream_pressure << "," << sd.ox.venturi_throat_pressure << ","
                << sd.ox.venturi_temperature << "," << sd.ox.valve_temperature << ","
                << sd.ipa.tank_pressure << "," << sd.ipa.venturi_upstream_pressure << "," << sd.ipa.venturi_throat_pressure << ","
                << cs.chamber_pressure_controller_p_component << "," << cs.chamber_pressure_controller_i_component << ","
                << cs.lox_angle_controller_p_component << "," << cs.lox_angle_controller_i_component << ","
                << cs.ipa_angle_controller_p_component << "," << cs.ipa_angle_controller_i_component << ","
                << vc_state.measured_lox_mdot << "," << vc_state.measured_ipa_mdot << ","
                << vc_state.ol_lox_mdot << "," << vc_state.ol_ipa_mdot << "," << vc_state.ol_lox_angle << "," << vc_state.ol_ipa_angle;

  curveTelemCSV.print();
  odriveLogFile.println(curveTelemCSV.str);
  odriveLogFile.flush();
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
