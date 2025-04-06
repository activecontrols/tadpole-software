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
  print_labeled_sensor(" PT Slot 0: ", PT::pressure_0.getPressure(), " mV");
  print_labeled_sensor(" PT Slot 2: ", PT::pressure_2.getPressure(), " mV");
  print_labeled_sensor(" PT Slot 4: ", PT::pressure_4.getPressure(), " mV");
  print_labeled_sensor(" PT Slot 6: ", PT::pressure_6.getPressure(), " mV");
  print_labeled_sensor(" PT Slot 8: ", PT::pressure_8.getPressure(), " mV");
  print_labeled_sensor("PT Slot 10: ", PT::pressure_10.getPressure(), " mV");
  print_labeled_sensor("PT Slot 12: ", PT::pressure_12.getPressure(), " mV");

  // print_labeled_sensor("           TC LOX Valve: ", TC::lox_valve_temperature.getTemperature_F(), " F");
  // print_labeled_sensor("         TC LOX Venturi: ", TC::lox_venturi_temperature.getTemperature_F(), " F");
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
  while (true) {
    print_all_sensors();
    delay(500);
  }
}

void loop() {
  Router::run(); // loop only runs once, since there is an internal loop in Router::run()
}