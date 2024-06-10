#include <Arduino.h>
#include "../lib/serial_comms/router.h"
#include "../lib/serial_comms/loader.h"

void ping() {
    char msg[] = "pong";
    Router::send(msg, sizeof(msg));
}

void setup() {
    Router::add({ping, "ping"}); // example registration
    Loader::begin(); // registers loader functions with the router
    Router::run(); // nothing runs after this. router handles everything
}

void loop() {}