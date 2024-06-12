//
// Created by Ishan Goel on 6/9/24.
//

#ifndef TADPOLE_SOFTWARE_ROUTER_H
#define TADPOLE_SOFTWARE_ROUTER_H

#include <vector>
#include <string>
#include <Wire.h>
#include <SD.h>

using namespace std;

#define COMMS_SERIAL Serial
#define COMMS_RATE 9600

struct func;

namespace Router {
//private:
//    // funcs is a vector of function pointers and their names
//    static vector<func> funcs;
//    static char logfilename[50];
//    static File logfile;
//    static bool logopen;
//
//    static void setLog();
//public:
    extern bool logenabled; // defaults to true
    // init_comms initializes the serial port
    extern void init_comms();
    // info sends a string & newline over serial
    extern void info(const char *msg);
    void info(String msg) { info(msg.c_str()); }
    void info(std::string msg) { info(msg.c_str()); }
    // log writes a string to the log file if logging is enabled
    extern void log(const char *msg);
    void log(String msg) { log(msg.c_str()); }
    void log(std::string msg) { log(msg.c_str()); }
    // send sends a message over the serial port. the caller is responsible for
    // freeing the memory of the message
    extern void send(char msg[], unsigned int len);
    // receive reads a message from the serial port into the supplied buffer.
    // the caller is responsible for freeing the memory of the message
    extern void receive(char msg[], unsigned int len);
    // add registers a new function to the Router
    extern void add(func f);
    // run starts monitoring the serial port for messages and calls the
    // appropriate function when a message is received. this function never
    // returns.
    [[noreturn]] extern void run();
};

struct func {
    void (*f)();
    const char *name;
};

#endif //TADPOLE_SOFTWARE_ROUTER_H
