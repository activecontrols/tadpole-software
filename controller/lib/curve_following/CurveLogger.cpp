#include "CurveLogger.h"

#include "pi_controller.h"
#include "CString.h"
#include "Driver.h"
#include "Router.h"
#include "SDCard.h"

namespace CurveLogger {

File odriveLogFile;
CString<400> curveTelemCSV;

#define LOG_HEADER ("time,lox_pos_cmd,"                                                                  \
                    "lox_pos,lox_vel,lox_voltage,lox_current,lox_temperature,"                           \
                    "water_tank_pressure,water_venturi_upstream_pressure,water_venturi_throat_pressure," \
                    "water_angle_controller_p_component,water_angle_controller_i_component,"             \
                    "ol_water_angle,mass_flow_estimate")

// logs time, phase, thrust, and sensor data in .csv format
void log_curve_csv(float time, Sensor_Data sd) {
  Controller_State cs = ClosedLoopControllers::getState();
  curveTelemCSV.clear();
  curveTelemCSV << time << "," << Driver::loxODrive.getLastPosCmd() << ","
                << Driver::loxODrive.getTelemetryCSV() << ","
                << sd.water.tank_pressure << "," << sd.water.venturi_upstream_pressure << "," << sd.water.venturi_throat_pressure << ","
                << cs.water_angle_controller_p_component << "," << cs.water_angle_controller_i_component << ","
                << ff_state.ol_water_angle << "," << ff_state.mass_flow_estimate;

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
