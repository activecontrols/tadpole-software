//
// Created by Ishan Goel on 6/9/24.
//

#include "Router.h"
#include <SDCard.h>

namespace Router {
    bool logenabled = true;

    namespace {
        vector<func> funcs;
        char logfilename[50] = "log.txt";
        File logfile;
        bool logopen = false;

        void setLog() {
            receive(logfilename, 49);
            logfilename[49] = '\0';
        }
    }

    // assumes sd inited fine.
    void log(const char *msg) {
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
                info("COULDN'T OPEN LOG FILE. LOGGING DISABLED.");
                logenabled = false;
                log(""); // trigger the close asap
                return;
            }
            logopen = true;
        }
        logfile.println(msg);
    }

    void info(const char *msg) {
        COMMS_SERIAL.println(msg);
    }

    void send(char msg[], unsigned int len) {
        COMMS_SERIAL.write(msg, len);
    }

    void receive(char msg[], unsigned int len) {
        COMMS_SERIAL.readBytes(msg, len);
    }

    String read(unsigned int len) {
        //read until newline char or 200 characters (hopefully none of our funcs have names that long lol)
        String s = COMMS_SERIAL.readStringUntil('\n', len); 
        return s;
    }

    void add(func f) {
        funcs.push_back(f);
    }

    void init_comms() {
        COMMS_SERIAL.begin(COMMS_RATE);
        COMMS_SERIAL.setTimeout((unsigned long) -1); // wrap around to max long so we never time out
        add({setLog, "set_log"});
    }
    elapsedMillis ledTime = 0;
    bool ledOn = false;

    [[noreturn]] void run() { // attribute here enables dead-code warning & compiler optimization
        digitalWrite(LED_BUILTIN, LOW); //led indication that software is waiting for command
        String s = "";
        
        while (true) {
            s = read(200);
            s.trim(); //remove leading/trailing whitespace or newline
            Router::info(s);
            for (auto &f: funcs) {
                if (s.equals(f.name)) {
                    digitalWrite(LED_BUILTIN, HIGH); //led indication that command is running
                    f.f(); // call the function. it can decide to send, receive or whatever.
                    digitalWrite(LED_BUILTIN, LOW);
                    break;
                }
            }
        }
    }
}