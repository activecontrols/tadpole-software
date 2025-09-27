/*
 * Loader.h
 *
 *  Created on: 2024-06-10 by Ishan Goel
 *  Description: This file contains the declaration of the Loader class, which  provides functions to load configurations
 *  and curves from either serial communication or an SD card, as well as write configurations and curves to an SD card.
 *  The Loader class also includes static variables to store the loaded control configuration, curve header,
 *  lerp points, and flags indicating whether a curve or configuration has been loaded.
 */

#ifndef TADPOLE_SOFTWARE_LOADER_H
#define TADPOLE_SOFTWARE_LOADER_H

#include <SD.h>

class Loader
{
public:
  static void begin(); // registers loader functions with the router
  Loader() = delete;   // prevent instantiation

  static void save_pt_zero();
  static void restore_pt_zero();
};

#endif // TADPOLE_SOFTWARE_LOADER_H