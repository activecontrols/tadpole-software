#include <Arduino.h>
#include <TeensyThreads.h>

#include "Driver.h"
#include "Router.h"
#include "Loader.h"
#include "SDCard.h"

void ping() {
   Router::info("pong");
}

void setup() {
    digitalWrite(LED_BUILTIN, HIGH);
    Router::init_comms();
    
    Router::info("Controller started.");
    Router::add({ping, "ping"}); // example registration
    if (!SDCard::begin()) {
        Router::logenabled = false;
        Router::info("SD card not found. SD logging disabled.");
    }

    Loader::begin(); // registers data loader functions with the router

    Driver::begin(); // initializes the odrives and functions to start curves

}

void loop() {
    Router::run(); //loop only runs once, since there is an internal loop in Router::run()
}