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
#include <Curve.h>

class Loader {
public:
  static curve_header header;
  static lerp_point_angle *lerp_angle_curve;
  static lerp_point_thrust *lerp_thrust_curve;
  static bool loaded_curve;

  static void begin(); // registers loader functions with the router
  Loader() = delete;   // prevent instantiation
  static bool load_curve_sd(const char *filename);

  static void save_pt_zero();
  static void restore_pt_zero();

private:
  // triggered by comms
  static void
  load_curve_serial();
  static void load_curve_sd_cmd();
  static void write_curve_sd();

  static void load_curve_generic(bool serial, File *f);
};

#endif // TADPOLE_SOFTWARE_LOADER_H