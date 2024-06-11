//
// Created by Ishan Goel on 6/10/24.
//

#include "Loader.h"
#include "../serial_comms/Router.h"
#include <SDCard.h>

control_config Loader::config;
curve_header Loader::header;
lerp_point_open* Loader::los;
lerp_point_closed* Loader::lcs;
bool Loader::loaded_curve;
bool Loader::loaded_config;

void Loader::begin() {
    Router::add({load_curve_serial, "load_curve_serial"});
    Router::add({load_config_serial, "load_config_serial"});
    Router::add({load_curve_sd, "load_curve_sd"});
    Router::add({load_config_sd, "load_config_sd"});
}

void Loader::load_curve_generic(bool serial, File* f) {
    auto receive = [=](char *buf, unsigned int len) {
        if (serial) {
            Router::receive(buf, len);
        } else {
            f->read(buf, len);
        }
    };
    receive((char *) &header, sizeof(header));
    switch (header.ctype) {
        case curve_type::sine:
        case curve_type::chirp:
            break; // params already present in header
        case curve_type::lerp:
            // load lerp points in from serial
            if (header.lerp.is_open) {
                los = (lerp_point_open *) (extmem_calloc(header.lerp.num_points, sizeof(lerp_point_open)));
                receive((char *) &los, sizeof(lerp_point_open) * header.lerp.num_points);
            } else {
                lcs = (lerp_point_closed *) (extmem_calloc(header.lerp.num_points, sizeof(lerp_point_closed)));
                receive((char *) &lcs, sizeof(lerp_point_closed) * header.lerp.num_points);
            }
            break;
    }
    loaded_curve = true;
}

void Loader::load_curve_serial() {
    load_curve_generic(true, nullptr);
    Router::send("Loaded curve!");
}

void Loader::load_config_serial() {
    Router::receive((char *) &config, sizeof(config));
    loaded_config = true;
    Router::send("Loaded config!");
}

void Loader::load_curve_sd() {
    char filename[50];
    Router::receive(filename, 50);
    File f = SDCard::open(filename, FILE_READ);
    if (f) {
        load_curve_generic(false, &f);
        f.close();
    } else {
        Router::send("File not found.");
        return;
    }
    Router::send("Loaded curve!");
}

void Loader::load_config_sd() {
    char filename[50];
    Router::receive(filename, 50);
    File f = SDCard::open(filename, FILE_READ);
    if (f) {
        f.read((char *) &config, sizeof(config));
        f.close();
    } else {
        Router::send("File not found.");
        return;
    }
    loaded_config = true;
    Router::send("Loaded config!");
}

// todo: implement way to write to sd card

