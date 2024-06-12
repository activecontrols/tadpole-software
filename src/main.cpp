#include <Arduino.h>
#include "Driver.h"
#include "Router.h"
#include "Loader.h"
#include "SDCard.h"

#define ENABLE_ODRIVE_COMM true

//void ping() {
//    Router::info("pong");
//}

void setup() {
    Router::init_comms();
//    Router::add({ping, "ping"}); // example registration
    if (!SDCard::begin()) {
        Router::logenabled = false;
        Router::info("SD card not found. Logging disabled.");
    }

    Loader::begin(); // registers data loader functions with the router

#if (ENABLE_ODRIVE_COMM)
    Driver::begin(); // initializes the odrives and functions to start curves
#endif

    Router::run(); // nothing runs after this. router handles everything
}

// code never reaches here because of Router::run()
void loop() {}