#include <Arduino.h>

#include "ZucrowInterface.h"
#include "CurveFollower.h"
#include "PressureSensor.h"
#include "Thermocouples.h"
#include "SPI_Demux.h"
#include "Driver.h"
#include "Router.h"
#include "Loader.h"
#include "Safety.h"

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
  print_labeled_sensor("            PT WATER Tank: ", PT::water_tank.getPressure(), " psi");
  print_labeled_sensor("PT WATER Venturi Upstream: ", PT::water_venturi_upstream.getPressure(), " psi");
  print_labeled_sensor("  PT WATER Venturi Throat: ", PT::water_venturi_throat.getPressure(), " psi");
  print_labeled_sensor("                 TC WATER: ", TC::water.getTemperature_F(), " F");
}

void setup() {
  Router::begin();
  Router::info("Controller started.");

  Router::add({ping, "ping"}); // example registration
  Router::add({help, "help"});
  Router::add({print_all_sensors, "print_sensors"});

  Safety::begin();          // prints safety info
  SPI_Demux::begin();       // initializes the SPI backplane
  Loader::begin();          // registers data loader functions with the router
  Driver::begin();          // initializes the odrives
  ZucrowInterface::begin(); // initializes the DAC
  PT::begin();              // initializes the PT Boards
  TC::begin();              // initializes the TC Boards
  CurveFollower::begin();   // creates curve following commands

  // CurveFollower::auto_seq();
}

void loop() {
  Router::run(); // loop only runs once, since there is an internal loop in Router::run()
}