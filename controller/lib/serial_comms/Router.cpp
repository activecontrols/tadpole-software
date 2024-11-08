//
// Created by Ishan Goel on 6/9/24.
// Maintained by Ishan Goel and Vincent Palmerio
//

#include "Router.h"

#include "SDCard.h"
#include "CString.h"

#define COMMAND_BUFFER_SIZE (200)

namespace Router {
    bool logenabled = true;

    CString<COMMAND_BUFFER_SIZE> commandBuffer;

    namespace {
        vector<func> funcs;
        char logfilename[50] = "log.txt";
        File logfile;
        bool logopen = false;

        void setLog() {
            receive(logfilename, 49);
            logfilename[49] = '\0';
        }

        void readCommand() {
            //read until newline char or 200 characters (hopefully none of our funcs have names that long lol)
            COMMS_SERIAL.readBytesUntil('\n', commandBuffer.str, COMMAND_BUFFER_SIZE - 1);
            commandBuffer.str[COMMAND_BUFFER_SIZE - 1] = '\0'; //null terminate
            commandBuffer.trim(); //remove leading/trailing whitespace or newline
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

    void receive_prompt(const char* prompt, char msg[], unsigned int len) {
        info(prompt);
        receive(msg, len);
    }

    String read(unsigned int len) {
        String s = COMMS_SERIAL.readStringUntil('\n', len); 
        s.trim(); //remove leading/trailing whitespace or newline
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

    [[noreturn]] void run() { // attribute here enables dead-code warning & compiler optimization
        digitalWrite(LED_BUILTIN, LOW); //led indication that software is waiting for command
        
        while (true) {
            readCommand();
            
            // commandBuffer.print(); // not needed with echo

            bool cmd_found = false;
            for (auto &f: funcs) {
                if (commandBuffer.equals(f.name)) {
                    digitalWrite(LED_BUILTIN, HIGH); //led indication that command is running
                    f.f(); // call the function. it can decide to send, receive or whatever.
                    digitalWrite(LED_BUILTIN, LOW);
                    cmd_found = true;
                    break;
                }
            }
            if (!cmd_found) {
                info("Command not found.");
            }
        }
    }

    void print_all_cmds() {
      info("All commands: ");
      for (auto &f : funcs) {
        info(f.name);
      }
    }
}