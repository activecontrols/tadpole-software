#include <Arduino.h>
#include "../lib/odrive/ODrive.h"
#include "../lib/serial_comms/Router.h"
#include "../lib/data/Loader.h"

#define ENABLE_ODRIVE_COMM false

void ping() {
    Router::info("pong");
}

void setup() {
    Router::add({ping, "ping"}); // example registration
    Loader::begin(); // registers loader functions with the router
#if (ENABLE_ODRIVE_COMM)
    ODrive::setupODrives(); // sets up the LOX and Fuel odrives
#endif
    Router::run(); // nothing runs after this. router handles everything
}

void loop() {}