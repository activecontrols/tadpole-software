#include "LogWriter.h"

#include "CString.h"
#include "Router.h"
#include "SDCard.h"

namespace LogWriter {

File odriveLogFile;
CString<400> csv_line;

#define LOG_HEADER "time,upstream,downstream,throat"

// logs time, phase, thrust, and sensor data in .csv format
int print_counter = 0;
void log_csv_data(float time, Sensor_Data sd, VC_State vc_state) {
  csv_line.clear();
  csv_line << time << ","
           << sd.upstream << ","
           << sd.downstream << ","
           << sd.throat;

  odriveLogFile.println(csv_line.str);
  odriveLogFile.flush();

  print_counter++;
  if (print_counter % 10 == 0) {
    csv_line.print();
  }
}

// creates a log file and prints csv header
void create_data_log(const char *filename) {
  odriveLogFile = SDCard::open(filename, FILE_WRITE);
  odriveLogFile.println(LOG_HEADER);
}

// close and flush the log file
void close_data_log() {
  odriveLogFile.flush();
  odriveLogFile.close();
}

} // namespace LogWriter
