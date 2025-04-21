/*
 * SDCard.h
 *
 *  Created on: 2024-06-11 by Ishan Goel
 *  Description: This file contains the declaration of the SDCard class, which provides functions for
 *  interacting with the SD card if one is loaded onto the Teensy.
 */

#ifndef TADPOLE_SOFTWARE_SDCARD_H
#define TADPOLE_SOFTWARE_SDCARD_H

#include <SD.h>

class SDCard {
public:
  static boolean begin();
  static File open(const char *filename, char mode);
  static String get_next_safe_name(const char *filename);

private:
  // used by comms
  static void ls();
  static void rm();
  static void cat();
  static void auto_cat();
};

#endif // TADPOLE_SOFTWARE_SDCARD_H
