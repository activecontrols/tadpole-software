#include <Arduino.h>
#include <ODrive.h>
#include "../lib/serial_comms/Router.h"
#include "../lib/data/Loader.h"

#define ENABLE_ODRIVE_COMM false

void ping() {
    char msg[] = "pong";
    Router::send(msg, sizeof(msg));
}

void setup() {
    Router::add({ping, "ping"}); // example registration
    Loader::begin(); // registers loader functions with the router
#if (ENABLE_ODRIVE_COMM)
    ODriveController::setupODrives(); // sets up the LOX and Fuel odrives
#endif
    Router::run(); // nothing runs after this. router handles everything
}

void loop() {}