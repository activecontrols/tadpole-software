//
// Created by Ishan Goel on 6/9/24.
//

#include "router.h"

vector<func> Router::funcs;

void Router::send(char msg[], unsigned int len) {
    COMMS_SERIAL.write(msg, len);
}

void Router::receive(char msg[], unsigned int len) {
    COMMS_SERIAL.readBytes(msg, len);
}

void Router::add(func f) {
    if (funcs.empty()) {
        init(); // nice way to avoid having to call init() in main
    }
    funcs.push_back(f);
}

void Router::init() {
    COMMS_SERIAL.begin(COMMS_RATE);
    COMMS_SERIAL.setTimeout((unsigned long) -1); // wrap around to max long so we never time out
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