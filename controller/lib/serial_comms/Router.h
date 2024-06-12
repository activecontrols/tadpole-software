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
    void init_comms();
    // info sends a string & newline over serial
    void info(const char *msg);
    inline void info(const String& msg) { info(msg.c_str()); }
    inline void info(const std::string& msg) { info(msg.c_str()); }
    // log writes a string to the log file if logging is enabled
    void log(const char *msg);
    inline void log(const String& msg) { log(msg.c_str()); }
    inline void log(const std::string& msg) { log(msg.c_str()); }
    // send sends a message over the serial port. the caller is responsible for
    // freeing the memory of the message
    void send(char msg[], unsigned int len);
    // receive reads a message from the serial port into the supplied buffer.
    // the caller is responsible for freeing the memory of the message
    void receive(char msg[], unsigned int len);
    // add registers a new function to the router
    void add(func f);
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
