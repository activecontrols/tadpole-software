#include <Arduino.h>

#include "Driver.h"
#include "Router.h"
#include "Loader.h"
#include "SDCard.h"
#include "spi_demux.hpp"
#include "zucrow_interface.hpp"
#include "PressureSensor.h"
#include "Thermocouples.h"

void ping() {
  Router::info("pong");
}

void help() {
  Router::print_all_cmds();
}

void print_labeled_sensor(const char *msg, float sensor_value, const char *unit) {
  Router::info_no_newline(msg);
  Router::info_no_newline(sensor_value);
  Router::info(unit);
}

void print_all_sensors() {
  Router::info("\n Sensor Status ");
  print_labeled_sensor("PT LOX In:   ", Pressure::lox_pressure_in.getPressure(), " psi");
  print_labeled_sensor("PT LOX Out:  ", Pressure::lox_pressure_out.getPressure(), " psi");
  print_labeled_sensor("TC 1:        ", TC::example_tc.getTemperature(), " F");
}

void auto_seq() {
  const char *curve_file_name = "AUTOCUR";
  const char *tpl_log_file_name = "AUTOL"; // generates names like AUTOL#.CSV
  Loader::load_curve_sd(curve_file_name);
  String log_file_name = SDCard::get_next_safe_name(tpl_log_file_name);
  Router::info_no_newline("Using log file: ");
  Router::info(log_file_name);
  Driver::createCurveLog(log_file_name.c_str());
  Driver::followCurve();
}

void setup() {
  Router::init_comms();
  Router::info("Controller started.");

  Router::add({ping, "ping"}); // example registration
  Router::add({help, "help"});
  Router::add({print_all_sensors, "print_sensors"});
  if (!SDCard::begin()) {
    Router::logenabled = false;
    Router::info("SD card not found. SD logging disabled.");
  }

  SPI_Demux::begin(); // initializes the SPI backplane

  Loader::begin(); // registers data loader functions with the router

  Driver::begin(); // initializes the odrives and functions to start curves

  ZucrowInterface::begin(); // initializes the DAC
  Pressure::begin();        // initializes the PT Boards
  TC::begin();              // initializes the TC Boards
  // auto_seq();
  Router::add({auto_seq, "auto_seq"});
  // Router::add({SPI_Demux::select_chip_cmd, "spi_select"});
  // Router::add({SPI_Demux::deselect_chip_cmd, "spi_deselect"});
}

void loop() {
  Router::run(); // loop only runs once, since there is an internal loop in Router::run()
}