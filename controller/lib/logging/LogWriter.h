#ifndef LOG_WRITER_H
#define LOG_WRITER_H

#include "valve_controller.h"

namespace LogWriter {
void create_data_log(const char *filename);
void log_csv_data(float time, Sensor_Data sd);
void close_data_log();

}; // namespace LogWriter

#endif
