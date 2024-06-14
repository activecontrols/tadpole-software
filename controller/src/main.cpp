#include <Arduino.h>
#include "Driver.h"
#include "Router.h"
#include "Loader.h"
#include "SDCard.h"

void ping() {
   Router::info("pong");
}

void setup() {
    Router::init_comms();
    Router::add({ping, "ping"}); // example registration
    if (!SDCard::begin()) {
        Router::logenabled = false;
        Router::info("SD card not found. SD logging disabled.");
    }

    Loader::begin(); // registers data loader functions with the router

    Driver::begin(); // initializes the odrives and functions to start curves

    Router::run(); // nothing runs after this. router handles everything
}

// code never reaches here because of Router::run()
void loop() {}