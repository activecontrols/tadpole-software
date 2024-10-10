#include <Arduino.h>
#include <TeensyThreads.h>

#include "Router.h"
#include "SDCard.h"

#ifdef ONLY_TEENSY_PRIMARY
#include "Driver.h"
#include "Loader.h"
#endif

#ifdef ONLY_TEENSY_SECONDARY
#include "TadpoleGimbal.hpp"
#endif

void ping() {
  Router::info("pong");
}

void setup() {
  digitalWrite(LED_BUILTIN, HIGH);
  Router::init_comms();
  Router::info("Controller started.");

  // Learn more about TeensyThreads lib here: https://github.com/ftrias/TeensyThreads

  // set current thread to a max time of 20 ms on the CPU before switching to another thread
  threads.setSliceMillis(20);

  // set all new threads to run on the CPU for a max of 20 ms before switching to another thread
  threads.setDefaultTimeSlice(20);

  Router::add({ping, "ping"}); // example registration
  if (!SDCard::begin()) {
    Router::logenabled = false;
    Router::info("SD card not found. SD logging disabled.");
  }

#ifdef ONLY_TEENSY_PRIMARY
  Router::info("Compiled for TEENSY PRIMARY");
  Loader::begin(); // registers data loader functions with the router
  Driver::begin(); // initializes the odrives and functions to start curves
#endif

#ifdef ONLY_TEENSY_SECONDARY
  Router::info("Compiled for TEENSY SECONDARY");
  TadpoleGimbal::begin();
#endif
}

void loop() {
  Router::run(); // loop only runs once, since there is an internal loop in Router::run()
}