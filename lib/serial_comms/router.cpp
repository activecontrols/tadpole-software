//
// Created by Ishan Goel on 6/9/24.
//

#include "router.h"

vector<func> router::funcs;

void router::send(char msg[], int len) {
    COMMS_SERIAL.write(msg, len);
}

void router::receive(char msg[], int len) {
    COMMS_SERIAL.readBytes(msg, len);
}

void router::add(func f) {
    if (funcs.empty()) {
        init(); // nice way to avoid having to call init() in main
    }
    funcs.push_back(f);
}

void router::init() {
    COMMS_SERIAL.begin(COMMS_RATE);
    COMMS_SERIAL.setTimeout((unsigned long) -1); // wrap around to max long so we never time out
}

[[noreturn]] void router::run() { // attribute here enables dead-code warning & compiler optimization
    while (true) {
        String s = COMMS_SERIAL.readStringUntil(0, 200); // read until null terminator or 200 characters
        for (auto &f : funcs) {                                   // (one would hope none of our functions have names that long lol)
            if (s.equals(f.name)) {
                f.f(); // call the function. it can decide to send, receive or whatever.
            }
        }
    }
}