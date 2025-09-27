//
// Created by Ishan Goel on 6/9/24.
// Maintained by Ishan Goel and Vincent Palmerio
//

#include "Router.h"

#include "CString.h"
#include "SDCard.h"

#define COMMAND_BUFFER_SIZE (200)

namespace Router {
File comms_log_file;

CString<COMMAND_BUFFER_SIZE> commandBuffer;

namespace {
vector<func> funcs;

void readCommand() {
  // read until newline char or 200 characters (hopefully none of our funcs have names that long lol)
  COMMS_SERIAL.readBytesUntil('\n', commandBuffer.str, COMMAND_BUFFER_SIZE - 1);
  commandBuffer.str[COMMAND_BUFFER_SIZE - 1] = '\0'; // null terminate
  commandBuffer.trim();                              // remove leading/trailing whitespace or newline

  comms_log_file.print("<");
  comms_log_file.print(commandBuffer.str);
  comms_log_file.print(">\n");
  comms_log_file.flush();
}
} // namespace

void begin() {
  COMMS_SERIAL.begin(COMMS_RATE);
  COMMS_SERIAL.setTimeout((unsigned long)-1); // wrap around to max long so we never time out

  if (SDCard::begin()) {
    comms_log_file = SDCard::open("log.txt", FILE_WRITE);
  } else {
    Router::info("SD card not found.");
    while (true) {
      Router::info("Reboot once SD card inserted...");
      delay(1000);
    }
  }
}

void info(const char *msg) {
  COMMS_SERIAL.println(msg);
  comms_log_file.println(msg);
  comms_log_file.flush();
}

void info_no_newline(const char *msg) {
  COMMS_SERIAL.print(msg);
  comms_log_file.print(msg);
  comms_log_file.flush();
}

void send(char msg[], unsigned int len) {
  COMMS_SERIAL.write(msg, len);
}

void receive(char msg[], unsigned int len) {
  COMMS_SERIAL.readBytes(msg, len);
}

String read(unsigned int len) {
  String s = COMMS_SERIAL.readStringUntil('\n', len);
  s.trim(); // remove leading/trailing whitespace or newline

  comms_log_file.print("<");
  comms_log_file.print(s);
  comms_log_file.print(">\n");
  comms_log_file.flush();

  return s;
}

void add(func f) {
  funcs.push_back(f);
}

[[noreturn]] void run() { // attribute here enables dead-code warning & compiler optimization

  while (true) {
    readCommand();

    // commandBuffer.print(); // not needed with echo

    bool cmd_found = false;
    for (auto &f : funcs) {
      if (commandBuffer.equals(f.name)) {
        f.f(); // call the function. it can decide to send, receive or whatever.
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
} // namespace Router