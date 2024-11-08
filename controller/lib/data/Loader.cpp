//
// Created by Ishan Goel on 6/10/24.
//

#include "Loader.h"
#include "../serial_comms/Router.h"
#include <SDCard.h>

control_config Loader::config;
curve_header Loader::header;
lerp_point_angle *Loader::lerp_angle_curve;
lerp_point_thrust *Loader::lerp_thrust_curve;
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

void Loader::load_curve_generic(bool serial, File *f) {

  if (loaded_curve) {
    free(lerp_angle_curve);
    lerp_angle_curve = NULL;
    free(lerp_thrust_curve);
    lerp_thrust_curve = NULL;
  }

  auto receive = [=](char *buf, unsigned int len) {
    if (serial)
      Router::receive(buf, len);
    else
      f->read(buf, len);
  };

  if (header.version != CURRENT_CURVEH_VERSION) {
    Router::info("ERROR! Attempted to load a curve from an older version. Aborting.");
    return;
  }

  receive((char *)&header, sizeof(header));
  switch (header.ctype) {
  case curve_type::sine:
  case curve_type::chirp:
    break; // params already present in header
  case curve_type::lerp:
    // load lerp points in from serial
    if (header.is_thrust) {
      lerp_thrust_curve = (lerp_point_thrust *)(extmem_calloc(header.lerp.num_points, sizeof(lerp_point_thrust)));
      receive((char *)lerp_thrust_curve, sizeof(lerp_point_thrust) * header.lerp.num_points);
    } else {
      lerp_angle_curve = (lerp_point_angle *)(extmem_calloc(header.lerp.num_points, sizeof(lerp_point_angle)));
      receive((char *)lerp_angle_curve, sizeof(lerp_point_angle) * header.lerp.num_points);
    }
    Serial.print("Loaded curve with: ");
    Serial.print(header.lerp.num_points);
    Serial.println(" points");

    for (int i = 0; i < header.lerp.num_points; i++) {
      Serial.print("Point: ");
      if (header.is_thrust) {
        Serial.print(lerp_thrust_curve[i].time);
        Serial.print(" sec | ");
        Serial.print(lerp_thrust_curve[i].thrust);
        Serial.println(" lbf.");
      } else {
        Serial.print(lerp_angle_curve[i].time);
        Serial.print(" sec | IPA ");
        Serial.print(lerp_angle_curve[i].ipa_angle);
        Serial.print(" deg | OX ");
        Serial.print(lerp_angle_curve[i].lox_angle);
        Serial.print(" deg.");
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
  Router::receive((char *)&config, sizeof(config));
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
    f.read((char *)&config, sizeof(config));
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
  f.write((char *)&header, sizeof(header));
  switch (header.ctype) {
  case curve_type::sine:
  case curve_type::chirp:
    break; // params already present in header
  case curve_type::lerp:
    if (header.is_thrust) {
      f.write((char *)lerp_thrust_curve, sizeof(lerp_point_thrust) * header.lerp.num_points);
    } else {
      f.write((char *)lerp_angle_curve, sizeof(lerp_point_angle) * header.lerp.num_points);
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
  } else {
    Router::info("File found.");
  }
  f.write((char *)&config, sizeof(config));
  f.close();
  Router::info("Wrote config!");
}
