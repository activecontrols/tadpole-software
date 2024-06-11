#include <Arduino.h>
#include <ODrive.h>
#include "../lib/serial_comms/Router.h"
#include "../lib/data/Loader.h"

void ping() {
    char msg[] = "pong";
    Router::send(msg, sizeof(msg));
}

void setup() {
    Router::add({ping, "ping"}); // example registration
    Loader::begin(); // registers loader functions with the router
    ODriveController::setupODrives(); // sets up the LOX and Fuel odrives
    Router::run(); // nothing runs after this. router handles everything
}

void loop() {}