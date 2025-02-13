/*
    * Router.h
    *
    *  Created on: 2024-06-09 by Ishan Goel
    *  Description: This file contains the declaration of the Router namespace and functionality 
    *  to initialize the serial port, send and receive messages, log messages, and register functions 
    *  to be called based on received messages.
*/

#ifndef TADPOLE_SOFTWARE_ROUTER_H
#define TADPOLE_SOFTWARE_ROUTER_H

#include <vector>
#include <string>
#include <functional>
#include <Wire.h>
#include <SD.h>

using namespace std;

#define COMMS_SERIAL Serial
#define COMMS_RATE 9600

struct func;

namespace Router {

    extern bool logenabled; // defaults to true
    // init_comms initializes the serial port
    void init_comms();
    // info sends a string & newline over serial
    void info(const char *msg);
    void info_no_newline(const char *msg);
	inline void info(const String &msg) { info(msg.c_str()); }
	inline void info_no_newline(const String &msg) { info_no_newline(msg.c_str()); }
    inline void info(const std::string& msg) { info(msg.c_str()); }
    inline void info_no_newline(const std::string &msg) { info_no_newline(msg.c_str()); }
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
    // receive reads a message from the serial port into the supplied buffer - prints prompt first.
    // the caller is responsible for freeing the memory of the message
    void receive_prompt(const char *prompt, char msg[], unsigned int len);
    //reads a message from the serial port into a string and returns it
    String read(unsigned int len);
    // add registers a new function to the router
    void add(func f);
    // run starts monitoring the serial port for messages and calls the
    // appropriate function when a message is received. this function never
    // returns.
    [[noreturn]] extern void run();
    void print_all_cmds();

};

struct func {
    std::function<void()> f;
    const char *name;
};

#endif //TADPOLE_SOFTWARE_ROUTER_H
