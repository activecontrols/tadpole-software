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
  print_labeled_sensor("            PT LOX Tank: ", PT::lox_tank.getPressure(), " psi");
  print_labeled_sensor("PT LOX Venturi Upstream: ", PT::lox_venturi_upstream.getPressure(), " psi");
  print_labeled_sensor("  PT LOX Venturi Throat: ", PT::lox_venturi_throat.getPressure(), " psi");

  print_labeled_sensor("            PT IPA Tank: ", PT::ipa_tank.getPressure(), " psi");
  print_labeled_sensor("PT IPA Venturi Upstream: ", PT::ipa_venturi_upstream.getPressure(), " psi");
  print_labeled_sensor("  PT IPA Venturi Throat: ", PT::ipa_venturi_throat.getPressure(), " psi");

  print_labeled_sensor("             PT Chamber: ", PT::chamber.getPressure(), " psi");

  print_labeled_sensor("           TC LOX Valve: ", TC::lox_valve_temperature.getTemperature_F(), " F");
  print_labeled_sensor("         TC LOX Venturi: ", TC::lox_venturi_temperature.getTemperature_F(), " F");
}

void auto_seq() { // TODO - home valves?
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
  PT::begin();              // initializes the PT Boards
  TC::begin();              // initializes the TC Boards
  // auto_seq();
  Router::add({auto_seq, "auto_seq"});
}

void loop() {
  Router::run(); // loop only runs once, since there is an internal loop in Router::run()
}