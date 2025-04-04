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

/**
 * Logs the telemetry data for a curve in CSV format.
 * @param time The elapsed time in seconds.
 * @param phase The current phase of the curve.
 * @param thrust The thrust value (for closed lerp curves) or -1 (for other curve types).
 */
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

/**
 * Creates a log file for the current curve.
 * @return The created log file.
 */
void create_curve_log(const char *filename) {
  odriveLogFile = SDCard::open(filename, FILE_WRITE);
  odriveLogFile.println(LOG_HEADER);
}

void close_curve_log() {
  odriveLogFile.flush();
  odriveLogFile.close();
}

} // namespace CurveLogger
