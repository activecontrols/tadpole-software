#include <Arduino.h>
#include <TeensyThreads.h>

#include "Driver.h"
#include "Router.h"
#include "Loader.h"
#include "SDCard.h"
#include "Pressure.h"

using namespace Pressure;

void ping() {
   Router::info("pong");
}

void help() {
    Router::print_all_cmds();
}

void setup() {
    digitalWrite(LED_BUILTIN, HIGH);
    Router::init_comms();
    Router::info("Controller started.");

    //Learn more about TeensyThreads lib here: https://github.com/ftrias/TeensyThreads

    //set current thread to a max time of 20 ms on the CPU before switching to another thread
    threads.setSliceMillis(20); 

    //set all new threads to run on the CPU for a max of 20 ms before switching to another thread
    threads.setDefaultTimeSlice(20); 

    loxUpstreamPressure.startPressureCheckThread();

    Router::add({ping, "ping"}); // example registration
    Router::add({help, "help"});
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