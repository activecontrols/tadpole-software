//
// Created by Ishan Goel on 6/10/24.
//

#include "PressureSensor.h"
#include "Loader.h"
#include "Router.h"
#include <SDCard.h>

void Loader::begin()
{
  Router::add({save_pt_zero, "save_pt_zero"});
  Router::add({restore_pt_zero, "restore_pt_zero"});
}

int pt_zero_version = 2; // change this if the struct format changes
struct PT_zero
{
  float lox_valve_upstream;
  float lox_valve_downstream;
  float lox_venturi_differential;

  float ipa_valve_upstream;
  float ipa_valve_downstream;
  float ipa_venturi_differential;

  float chamber;
};

void Loader::save_pt_zero()
{
  PT_zero ptz;
  ptz.lox_valve_upstream = PT::lox_valve_upstream.offset;
  ptz.lox_valve_downstream = PT::lox_valve_downstream.offset;
  ptz.lox_venturi_differential = PT::lox_venturi_differential.offset;

  ptz.ipa_valve_upstream = PT::ipa_valve_upstream.offset;
  ptz.ipa_valve_downstream = PT::ipa_valve_downstream.offset;
  ptz.ipa_venturi_differential = PT::ipa_venturi_differential.offset;

  ptz.chamber = PT::chamber.offset;

  SD.remove("ptzero");
  File f = SDCard::open("ptzero", FILE_WRITE);
  f.write((char *)&pt_zero_version, sizeof(int));
  f.write((char *)&ptz, sizeof(PT_zero));
  f.close();
  Router::info("Saved pt zero.");
}

void Loader::restore_pt_zero()
{
  PT_zero ptz;

  int local_version;
  File f = SDCard::open("ptzero", FILE_READ);
  if (!f)
  {
    Router::info("Zero file not found.");
    return;
  }
  f.read((char *)&local_version, sizeof(int));
  if (local_version != pt_zero_version)
  {
    Router::info("Tried to load zero from incorrect version.");
    f.close();
    return;
  }
  f.read((char *)&ptz, sizeof(PT_zero));
  f.close();

  PT::lox_valve_upstream.offset = ptz.lox_valve_upstream;
  PT::lox_valve_downstream.offset = ptz.lox_valve_downstream;
  PT::lox_venturi_differential.offset = ptz.lox_venturi_differential;

  PT::ipa_valve_upstream.offset = ptz.ipa_valve_upstream;
  PT::ipa_valve_downstream.offset = ptz.ipa_valve_downstream;
  PT::ipa_venturi_differential.offset = ptz.ipa_venturi_differential;

  PT::chamber.offset = ptz.chamber;
  PT::zeroed_since_boot = true;

  Router::info("Restored pt zero.");
}