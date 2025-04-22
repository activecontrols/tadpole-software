#include "Arduino.h"

#define tadpole_AREA_OF_THROAT 1.69 // in^2
#define tadpole_C_STAR 4998.0654    // ft / s
#define GRAVITY_FT_S 32.1740        // Gravity in (ft / s^2)

#define R 82.1987
#define T_C 5261.370
#define V_C 67.3196

float Pc_plant_val = 0;
long long last_compute_time = 0;

void reset_plant() {
  pinMode(23, INPUT);
  Pc_plant_val = 0;
  last_compute_time = millis();
}

float chamber_pressure_plant(float mdot_ox, float mdot_ipa) {
  float dist = analogRead(23) / 512.0 - 1;
  long long this_compute_time = millis();
  float dp = (R * T_C / V_C) * (mdot_ox + mdot_ipa - (1 + dist * 0.1) * GRAVITY_FT_S * tadpole_AREA_OF_THROAT / tadpole_C_STAR * Pc_plant_val);
  Pc_plant_val += (this_compute_time - last_compute_time) * 0.001 * dp;
  last_compute_time = this_compute_time;
  return Pc_plant_val;
}