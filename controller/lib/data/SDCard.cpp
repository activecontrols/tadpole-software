//
// Created by Ishan Goel on 6/11/24.
//

#include <SD.h>
#include "SDCard.h"
#include "Router.h"

boolean SDCard::begin() {
  if (!SD.begin(BUILTIN_SDCARD))
    return false;
  Router::add({ls, "ls"});
  Router::add({rm, "rm"});
  //    Router::add({cat, "cat"});
  return true;
}

File SDCard::open(const char *filename, char mode) {
  return SD.open(filename, mode);
}

void SDCard::ls() {
  String result = "";
  File root = SD.open("/");
  while (true) {
    File entry = root.openNextFile();
    if (!entry) {
      break;
    }
    result += entry.name();
    result += " ";
    entry.close();
  }
  root.close();
  Router::info(result.c_str());
}

void SDCard::rm() {
  char filename[50];
  Router::receive(filename, 50);
  if (SD.remove(filename)) {
    Router::info("File removed.");
  } else {
    Router::info("File not found.");
  }
}

String SDCard::get_next_safe_name(const char *filename) {
  for (int i = 0; i < 100; i++) {
    String filename_str = filename;
    filename_str += i;
    filename_str += ".CSV";
    if (!SD.exists(filename_str.c_str())) {
      return filename_str.c_str();
    }
  }

  String filename_str = filename;
  filename_str += "_E.CSV";
  return filename_str;
}

// issue: receiver may not know when to stop reading. send size beforehand if absolutely needed.
// void SDCard::cat() {
//    char filename[50];
//    Router::receive(filename, 50);
//    File f = SD.open(filename, FILE_READ);
//    if (f) {
//        while (f.available()) {
//            Router::send((char *) f.read(), 1);
//        }
//        f.close();
//    } else {
//        Router::send("File not found.");
//    }
//}
