//
// Created by Ishan Goel on 6/9/24.
//

#ifndef TADPOLE_SOFTWARE_ROUTER_H
#define TADPOLE_SOFTWARE_ROUTER_H

#include <vector>
#include <Wire.h>

using namespace std;

#define COMMS_SERIAL Serial
#define COMMS_RATE 9600

struct func;

class Router {
private:
    // funcs is a vector of function pointers and their names
    static vector<func> funcs;
    static void init();
public:
    // send sends a message over the serial port. the caller is responsible for
    // freeing the memory of the message
    static void send(char msg[], unsigned int len);
    static void send(const char * msg);
    // receive reads a message from the serial port into the supplied buffer.
    // the caller is responsible for freeing the memory of the message
    static void receive(char msg[], unsigned int len);
    // add registers a new function to the Router
    static void add(func f);
    // run starts monitoring the serial port for messages and calls the
    // appropriate function when a message is received. this function never
    // returns.
    [[noreturn]] static void run();
    Router() = delete; // prevent instantiation
};

struct func {
    void (*f)();
    const char *name;
};

#endif //TADPOLE_SOFTWARE_ROUTER_H
