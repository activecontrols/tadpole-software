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