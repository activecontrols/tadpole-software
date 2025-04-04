#ifndef CURVE_LOGGER_H
#define CURVE_LOGGER_H

#include "valve_controller.h"

namespace CurveLogger {
void create_curve_log(const char *filename);
void log_curve_csv(float time, int phase, float thrust, Sensor_Data sd);
void close_curve_log();

}; // namespace CurveLogger

#endif
