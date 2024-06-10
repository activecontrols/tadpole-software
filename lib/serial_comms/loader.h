//
// Created by Ishan Goel on 6/10/24.
//

#ifndef TADPOLE_SOFTWARE_LOADER_H
#define TADPOLE_SOFTWARE_LOADER_H

typedef struct {
    float time;      // seconds since start
    float thrust;    // 0-100
    float lox_angle; // 0-90 degrees?
    float ipa_angle; // 0-90 degrees?
} waypoint;

typedef struct {
    char curve_label[50];
    int num_waypoints;
    char checksum[4];
} curve_header;

typedef struct {
    float lox_mdot_gains[30];
    float ipa_mdot_gains[30];
    float thrust_gains[30];
    float cf, // thrust coefficient
    cstar, // characteristic velocity
    cf_efficiency,
    cstar_efficiency,
    cd_ox, // venturi discharge coeff
    cd_ipa,
    at_ox_venturi, // throat area
    at_ipa_venturi, // throat area
    at_engine; // tadpole throat area
    // density vs temp. range is 0-29 degrees C
    float rho_ox_by_temp[30];
    // density vs temp. range is 0-29 degrees C
    float rho_ipa_by_temp[30];
    float lox_vapor_press_by_temp[30];
    bool loop_open;
} control_config; // all placeholders for now

#endif //TADPOLE_SOFTWARE_LOADER_H