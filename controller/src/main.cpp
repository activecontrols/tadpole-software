#include <TeensyThreads.h>
#include <Arduino.h>

ThreadWrap(Serial1, SerialXtra1);
#define Serial1 ThreadClone(SerialXtra1)

ThreadWrap(Serial2, SerialXtra2);
#define Serial2 ThreadClone(SerialXtra2)

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