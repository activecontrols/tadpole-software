//
// Created by Ishan Goel on 6/10/24.
//

#include "Loader.h"
#include "Router.h"
#include <SDCard.h>

curve_header Loader::header;
lerp_point_angle *Loader::lerp_angle_curve;
lerp_point_thrust *Loader::lerp_thrust_curve;
bool Loader::loaded_curve;

void Loader::begin() {
  Router::add({load_curve_serial, "load_curve_serial"});
  Router::add({load_curve_sd_cmd, "load_curve_sd"});
  Router::add({write_curve_sd, "write_curve_sd"});
}

void Loader::load_curve_generic(bool serial, File *f) {

  if (loaded_curve) {
    extmem_free(lerp_angle_curve);
    lerp_angle_curve = NULL;
    extmem_free(lerp_thrust_curve);
    lerp_thrust_curve = NULL;
  }

  auto receive = [=](char *buf, unsigned int len) {
    if (serial)
      Router::receive(buf, len);
    else
      f->read(buf, len);
  };

  receive((char *)&header, sizeof(header));

  if (header.version != CURRENT_CURVEH_VERSION) {
    Router::info("ERROR! Attempted to load a curve from an older version. Aborting.");
    return;
  }

  // load lerp points in from serial
  if (header.is_thrust) {
    lerp_thrust_curve = (lerp_point_thrust *)(extmem_calloc(header.num_points, sizeof(lerp_point_thrust)));
    receive((char *)lerp_thrust_curve, sizeof(lerp_point_thrust) * header.num_points);
  } else {
    lerp_angle_curve = (lerp_point_angle *)(extmem_calloc(header.num_points, sizeof(lerp_point_angle)));
    receive((char *)lerp_angle_curve, sizeof(lerp_point_angle) * header.num_points);
  }
  Router::info_no_newline("Loaded curve with: ");
  Router::info_no_newline(header.num_points);
  Router::info(" points");

  for (int i = 0; i < header.num_points; i++) {
    Router::info_no_newline("Point: ");
    if (header.is_thrust) {
      Router::info_no_newline(lerp_thrust_curve[i].time);
      Router::info_no_newline(" sec | ");
      Router::info_no_newline(lerp_thrust_curve[i].thrust);
      Router::info(" lbf.");
    } else {
      Router::info_no_newline(lerp_angle_curve[i].time);
      Router::info_no_newline(" sec | IPA ");
      Router::info_no_newline(lerp_angle_curve[i].ipa_angle);
      Router::info_no_newline(" deg | OX ");
      Router::info_no_newline(lerp_angle_curve[i].lox_angle);
      Router::info(" deg.");
    }
  }

  loaded_curve = true;
}

void Loader::load_curve_serial() {
  Router::info("Preparing to load curve!");
  load_curve_generic(true, nullptr);
  Router::info("Loaded curve!");
}

void Loader::load_curve_sd_cmd() {
  // filenames use DOS 8.3 standard
  Router::info_no_newline("Enter filename: ");
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

bool Loader::load_curve_sd(const char *filename) {
  File f = SDCard::open(filename, FILE_READ);
  if (f) {
    load_curve_generic(false, &f);
    f.close();
  } else {
    Router::info("File not found.");
    return false;
  }
  return loaded_curve;
}

void Loader::write_curve_sd() {
  // filenames use DOS 8.3 standard
  Router::info_no_newline("Enter filename: ");
  String filename = Router::read(50);
  File f = SDCard::open(filename.c_str(), FILE_WRITE);
  if (!f) {
    Router::info("File not found.");
    return;
  }
  f.write((char *)&header, sizeof(header));
  if (header.is_thrust) {
    f.write((char *)lerp_thrust_curve, sizeof(lerp_point_thrust) * header.num_points);
  } else {
    f.write((char *)lerp_angle_curve, sizeof(lerp_point_angle) * header.num_points);
  }

  f.close();
  Router::info("Wrote curve!");
}