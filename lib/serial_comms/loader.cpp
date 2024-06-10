//
// Created by Ishan Goel on 6/10/24.
//

#include "loader.h"
#include "router.h"

void Loader::begin() {
    Router::add({load_curve, "load_curve"});
    Router::add({load_config, "load_config"});
}

// todo: eventually store into some static variables that are used when we receive a start command
// todo: for that we need to store ctype and open/closed info as well.
void Loader::load_curve() {
    curve_header ch;
    Router::receive((char *) &ch, sizeof(ch));
    switch (ch.ctype) {
        case curve_type::sine:
            // do something with ch.sine_params
            break;
        case curve_type::chirp:
            // do something with ch.chirp_params
            break;
        case curve_type::lerp:
            // load lerp points
            if (ch.lerp_params.is_open) {
                // todo: statically alloced for now. probably need to change to dynamic
                lerp_point_open lpos[ch.lerp_params.num_waypoints];
                Router::receive((char *) &lpos, sizeof(lpos));
                // todo: do something with lpos
            } else {
                // todo: same as above
                lerp_point_closed lpcs[ch.lerp_params.num_waypoints];
                Router::receive((char *) &lpcs, sizeof(lpcs));
                // todo: do something with lpcs
            }
            break;
    }
}


