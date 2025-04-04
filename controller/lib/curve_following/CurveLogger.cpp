#include "CurveLogger.h"

#include "CString.h"
#include "Driver.h"
#include "Router.h"
#include "SDCard.h"

namespace CurveLogger {

File odriveLogFile;
CString<400> curveTelemCSV;

#define LOG_HEADER ("time,phase,thrust_cmd,lox_pos_cmd,ipa_pos_cmd,"           \
                    "lox_pos,lox_vel,lox_voltage,lox_current,lox_temperature," \
                    "ipa_pos,ipa_vel,ipa_voltage,ipa_current,ipa_temperature," \
                    "water_tank_pressure,water_venturi_upstream_pressure,water_venturi_throat_pressure")

// logs time, phase, thrust, and sensor data in .csv format
void log_curve_csv(float time, int phase, float thrust, Sensor_Data sd) {
  curveTelemCSV.clear();
  curveTelemCSV << time << "," << phase << "," << thrust << "," << Driver::loxODrive.getLastPosCmd() << "," << Driver::ipaODrive.getLastPosCmd() << ","
                << Driver::loxODrive.getTelemetryCSV() << "," << Driver::ipaODrive.getTelemetryCSV() << ","
                << sd.water.tank_pressure << "," << sd.water.venturi_upstream_pressure << "," << sd.water.venturi_throat_pressure;

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
