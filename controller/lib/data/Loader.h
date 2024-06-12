//
// Created by Ishan Goel on 6/10/24.
//

#ifndef TADPOLE_SOFTWARE_LOADER_H
#define TADPOLE_SOFTWARE_LOADER_H

#include <SD.h>
#include <Curve.h>

class Loader {
public:
    static control_config config;
    static curve_header header;
    static lerp_point_open* los;
    static lerp_point_closed* lcs;
    static bool loaded_curve;
    static bool loaded_config;

    static void begin(); // registers loader functions with the router
    Loader() = delete; // prevent instantiation
private:
    // triggered by comms
    static void load_curve_serial();
    static void load_config_serial();
    static void load_curve_sd();
    static void load_config_sd();
    static void write_curve_sd();
    static void write_config_sd();

    static void load_curve_generic(bool serial, File* f);
};

#endif //TADPOLE_SOFTWARE_LOADER_H