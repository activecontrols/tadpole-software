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
  Router::add({cat, "cat"});
  Router::add({auto_cat, "auto_cat"});
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
  Router::info_no_newline("Enter filename: ");
  String filename = Router::read(50);
  if (SD.remove(filename.c_str())) {
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
void SDCard::cat() {
  Router::info_no_newline("Enter filename: ");
  String filename = Router::read(50);
  File f = SD.open(filename.c_str(), FILE_READ);
  if (f) {
    while (f.available()) {
      Serial.println(f.readStringUntil('\n'));
    }
    f.close();
  } else {
    Router::info("File not found.");
  }
}

void SDCard::auto_cat() {
  Router::info_no_newline("Enter filename: ");
  String filename = Router::read(50);
  File f = SD.open(filename.c_str(), FILE_READ);
  if (f) {
    while (f.available()) {
      Serial.println(f.readStringUntil('\n'));
      Serial.readStringUntil('\n'); // wait for enter
    }
    f.close();
  } else {
    Router::info("File not found.");
  }
  Serial.println("done");
}