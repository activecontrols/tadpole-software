//
// Created by Ishan Goel on 6/9/24.
//

#include "Router.h"
#include <SDCard.h>

vector<func> Router::funcs;
char Router::logfilename[50] = "log.txt";
File Router::logfile;
bool Router::logenabled = true;
bool Router::logopen = false;


void Router::setLog() {
    Router::receive(logfilename, 50);
    logfilename[49] = 0; // just in case
}

// assumes sd inited fine.
void Router::log(const char *msg) {
    if (!logenabled) {
        if (logopen) {
            logfile.close();
            logopen = false;
        }
        return;
    }
    if (!logopen) {
        logfile = SD.open(logfilename, FILE_WRITE);
        if (!logfile) {
            Router::info("COULDN'T OPEN LOG FILE. LOGGING DISABLED.");
            logenabled = false;
            Router::log(""); // trigger the close asap
            return;
        }
        logopen = true;
    }
    logfile.println(msg);
}

void Router::info(const char *msg) {
    COMMS_SERIAL.println(msg);
}

void Router::send(char msg[], unsigned int len) {
    COMMS_SERIAL.write(msg, len);
}

void Router::receive(char msg[], unsigned int len) {
    COMMS_SERIAL.readBytes(msg, len);
}

void Router::add(func f) {
    funcs.push_back(f);
}

void Router::init_comms() {
    COMMS_SERIAL.begin(COMMS_RATE);
    COMMS_SERIAL.setTimeout((unsigned long) -1); // wrap around to max long so we never time out
    Router::add({setLog, "set_log"});
}

[[noreturn]] void Router::run() { // attribute here enables dead-code warning & compiler optimization
    String s = "";
    while (true) {
        s = COMMS_SERIAL.readStringUntil(0, 200); // read until null terminator or 200 characters (hopefully none of our funcs have names that long lol)
        for (auto &f : funcs) {
            if (s.equals(f.name)) {
                f.f(); // call the function. it can decide to send, receive or whatever.
            }
        }
    }
}