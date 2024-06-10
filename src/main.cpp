#include <Arduino.h>
#include "../lib/serial_comms/router.h"

void ping() {
    char msg[] = "pong";
    Router::send(msg, sizeof(msg));
}

void setup() {
    // put your setup code here, to run once:
    Router::add({ping, "ping"});
    Router::run();
}