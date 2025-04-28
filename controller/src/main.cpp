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

void setup() {
  Router::begin();
  Router::info("Controller started.");

  Router::add({ping, "ping"}); // example registration
  Router::add({help, "help"});

  Safety::begin();          // prints safety info
  SPI_Demux::begin();       // initializes the SPI backplane
  Loader::begin();          // registers data loader functions with the router
  Driver::begin();          // initializes the odrives
  ZucrowInterface::begin(); // initializes the DAC
  PT::begin();              // initializes the PT Boards
  TC::begin();              // initializes the TC Boards
  CurveFollower::begin();   // creates curve following commands
  ZucrowInterface::report_angles_for_five_seconds();
}

void loop() {
  Router::run(); // loop only runs once, since there is an internal loop in Router::run()
}