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
    Router::add({write_curve_sd, "write_curve_sd"});
    Router::add({write_config_sd, "write_config_sd"});
}

void Loader::load_curve_generic(bool serial, File* f) {

    if (loaded_curve) {
        free(los);
        los = NULL;
        free(lcs);
        lcs = NULL;
    }

    auto receive = [=](char *buf, unsigned int len) {
        if (serial) Router::receive(buf, len); else f->read(buf, len);
    };
    receive((char *) &header, sizeof(header));
    switch (header.ctype) {
        case curve_type::sine:
        case curve_type::chirp:
            break; // params already present in header
        case curve_type::lerp:
            // load lerp points in from serial
            if (header.is_open) {
                los = (lerp_point_open *) (extmem_calloc(header.lerp.num_points, sizeof(lerp_point_open)));
                receive((char *) &los, sizeof(lerp_point_open) * header.lerp.num_points);
            } else {
                lcs = (lerp_point_closed *) (extmem_calloc(header.lerp.num_points, sizeof(lerp_point_closed)));
                receive((char *) &lcs, sizeof(lerp_point_closed) * header.lerp.num_points);

                Serial.print("Loaded curve with: ");
                Serial.print(header.lerp.num_points);
                Serial.println(" points");
                
                for (int i = 0; i < header.lerp.num_points; i++) {
                  Serial.print("Point: ");
                  Serial.print(lcs[i].time);
                  Serial.print(" sec | ");
                  Serial.print(lcs[i].thrust);
                  Serial.println(" lbf.");
                }
            }
            break;
    }
    loaded_curve = true;
}

void Loader::load_curve_serial() {
    load_curve_generic(true, nullptr);
    Router::info("Loaded curve!");
}

void Loader::load_config_serial() {
    Router::receive((char *) &config, sizeof(config));
    loaded_config = true;
    Router::info("Loaded config!");
}

void Loader::load_curve_sd() {
    Router::info("Enter filename: ");
    String filename = Router::read(50);
    File f = SDCard::open(filename.c_str(), FILE_READ);
    if (f) {
        load_curve_generic(false, &f);
        f.close();
    } else {
        Router::info("File not found.");
        return;
    }
    Router::info("Loaded curve!");
}

void Loader::load_config_sd() {
    Router::info("Enter filename: ");
    String filename = Router::read(50);
    File f = SDCard::open(filename.c_str(), FILE_READ);
    if (f) {
        f.read((char *) &config, sizeof(config));
        f.close();
    } else {
        Router::info("File not found.");
        return;
    }
    loaded_config = true;
    Router::info("Loaded config!");
}

void Loader::write_curve_sd() {

    Router::info("Enter filename: ");
    String filename = Router::read(50);
    File f = SDCard::open(filename.c_str(), FILE_WRITE);
    if (!f) {
        Router::info("File not found.");
        return;
    }
    f.write((char *) &header, sizeof(header));
    switch (header.ctype) {
        case curve_type::sine:
        case curve_type::chirp:
            break; // params already present in header
        case curve_type::lerp:
            if (header.is_open) {
                f.write((char *) los, sizeof(lerp_point_open) * header.lerp.num_points);
            } else {
                f.write((char *) lcs, sizeof(lerp_point_closed) * header.lerp.num_points);
            }
            break;
    }
    f.close();
    Router::info("Wrote curve!");
}

void Loader::write_config_sd() {
    Router::info("Enter filename: ");
    String filename = Router::read(50);
    File f = SDCard::open(filename.c_str(), FILE_WRITE);
    if (!f) {
        Router::info("File not found.");
        return;
    }
    else {
        Router::info("File found.");
    }
    f.write((char *) &config, sizeof(config));
    f.close();
    Router::info("Wrote config!");
}
