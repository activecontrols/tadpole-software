//
// Created by Ishan Goel on 6/10/24.
//

#include "loader.h"
#include "router.h"

control_config Loader::config;
curve_header Loader::header;
lerp_point_open* Loader::los;
lerp_point_closed* Loader::lcs;
bool Loader::loaded_curve;
bool Loader::loaded_config;

void Loader::begin() {
    Router::add({load_curve, "load_curve"});
    Router::add({load_config, "load_config"});
}

// todo: eventually store into some static variables that are used when we receive a start command
// todo: for that we need to store ctype and open/closed info as well.
void Loader::load_curve() {
    Router::receive((char *) &header, sizeof(header));
    switch (header.ctype) {
        case curve_type::sine:
        case curve_type::chirp:
            break; // params already present in header
        case curve_type::lerp:
            // load lerp points in from serial
            if (header.lerp.is_open) {
                los = (lerp_point_open *) (extmem_calloc(header.lerp.num_points, sizeof(lerp_point_open)));
                Router::receive((char *) &los, sizeof(lerp_point_open) * header.lerp.num_points);
            } else {
                lcs = (lerp_point_closed *) (extmem_calloc(header.lerp.num_points, sizeof(lerp_point_closed)));
                Router::receive((char *) &lcs, sizeof(lerp_point_closed) * header.lerp.num_points);
            }
            break;
    }
    loaded_curve = true;
}

void Loader::load_config() {
    Router::receive((char *) &config, sizeof(config));
    loaded_config = true;
}

// todo: implement way to load from sd card

