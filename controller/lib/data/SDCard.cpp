//
// Created by Ishan Goel on 6/11/24.
//

#include <SD.h>
#include "SDCard.h"
#include "Router.h"

boolean SDCard::begin() {
    if (!SD.begin(BUILTIN_SDCARD)) return false;
    Router::add({ls, "ls"});
    Router::add({rm, "rm"});
//    Router::add({cat, "cat"});
    return true;
}

File SDCard::open(const char* filename, char mode) {
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

// issue: receiver may not know when to stop reading. send size beforehand if absolutely needed.
//void SDCard::cat() {
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
