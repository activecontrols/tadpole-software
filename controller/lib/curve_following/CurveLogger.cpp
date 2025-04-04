#include "CurveLogger.h"

#include "CString.h"
#include "Driver.h"
#include "Router.h"
#include "SDCard.h"

namespace CurveLogger {

File odriveLogFile;
CString<400> curveTelemCSV;

#define LOG_HEADER ("time,phase,thrust_cmd,lox_pos_cmd,ipa_pos_cmd,"                               \
                    "lox_pos,lox_vel,lox_voltage,lox_current,lox_temperature,"                     \
                    "ipa_pos,ipa_vel,ipa_voltage,ipa_current,ipa_temperature,"                     \
                    "lox_tank_pressure,lox_venturi_upstream_pressure,lox_venturi_throat_pressure," \
                    "lox_venturi_temperature,lox_valve_temperature,"                               \
                    "ipa_tank_pressure,ipa_venturi_upstream_pressure,ipa_venturi_throat_pressure")

// logs time, phase, thrust, and sensor data in .csv format
void log_curve_csv(float time, int phase, float thrust, Sensor_Data sd) {
  curveTelemCSV.clear();
  curveTelemCSV << time << "," << phase << "," << thrust << "," << Driver::loxODrive.getLastPosCmd() << "," << Driver::ipaODrive.getLastPosCmd() << ","
                << Driver::loxODrive.getTelemetryCSV() << "," << Driver::ipaODrive.getTelemetryCSV() << ","
                << sd.ox.tank_pressure << "," << sd.ox.venturi_upstream_pressure << "," << sd.ox.venturi_throat_pressure << ","
                << sd.ox.venturi_temperature << "," << sd.ox.valve_temperature << ","
                << sd.ipa.tank_pressure << "," << sd.ipa.venturi_upstream_pressure << "," << sd.ipa.venturi_throat_pressure;

  curveTelemCSV.print();
  odriveLogFile.println(curveTelemCSV.str);
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
