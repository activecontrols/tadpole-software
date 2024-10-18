#include <valve_controller.hpp>

// TODO - efficiency coeff?
#define tadpole_AREA_OF_THROAT 1.69      // in^2
#define tadpole_C_STAR 4998.0654         // ft / s
#define tadpole_MASS_FLOW_RATIO 1.2      // #ox = 1.2 * fuel
#define GRAVITY_FT_S 32.1740             // Gravity in ft / s^2
#define GRAVITY_IN_S (GRAVITY_FT_S * 12) // Gravity in in / s^2

#define cav_vent_ox_AREA_OF_THROAT 0.00989 // in^2
#define cav_vent_ox_CD 0.975               // unitless
#define fluid_ox_SPECIFIC_GRAVITY 1.15     // unitless
#define fluid_ox_VAPOUR_PRESSURE 14.2      // psi
#define fluid_ox_DENSITY 0.04146           // lb/in^3
#define fluid_ox_UPSTREAM_PRESSURE 820     // psi

#define cav_vent_ipa_AREA_OF_THROAT 0.00989 // in^2
#define cav_vent_ipa_CD 0.975               // unitless
#define fluid_ipa_SPECIFIC_GRAVITY 0.788    // unitless
#define fluid_ipa_VAPOUR_PRESSURE 0.638     // psi
#define fluid_ipa_DENSITY 0.02836           // lb/in^3
#define fluid_ipa_UPSTREAM_PRESSURE 820     // psi

#define INTERPOLATION_TABLE_LENGTH 20 // max length of all tables - set to enable passing tables to functions
#define VALVE_ANGLE_TABLE_LEN 11
float valve_angle_table[2][INTERPOLATION_TABLE_LENGTH] = {
    {0.000, 0.070, 0.161, 0.378, 0.670, 1.000, 1.450, 2.050, 2.780, 3.710, 4.960},
    {0, 9, 18, 27, 36, 45, 54, 63, 72, 81, 90}};
#define CF_THRUST_TABLE_LEN 2
float cf_thrust_table[2][INTERPOLATION_TABLE_LENGTH] = {
    {220, 550},
    {1.12, 1.3}};

struct fluid {
  float cav_vent_throat_area;
  float cav_vent_cd;
  float specific_gravity;
  float vapour_pressure;
  float density;
  float upstream_pressure;
};

fluid ox{cav_vent_ox_AREA_OF_THROAT, cav_vent_ox_CD, fluid_ox_SPECIFIC_GRAVITY, fluid_ox_VAPOUR_PRESSURE, fluid_ox_DENSITY, fluid_ox_UPSTREAM_PRESSURE};
fluid ipa{cav_vent_ipa_AREA_OF_THROAT, cav_vent_ipa_CD, fluid_ipa_SPECIFIC_GRAVITY, fluid_ipa_VAPOUR_PRESSURE, fluid_ipa_DENSITY, fluid_ox_UPSTREAM_PRESSURE};

// maps v from (min_in, max_in) to (min_out, max_out)
float linear_interpolation(float v, float min_in, float max_in, float min_out, float max_out) {
  return (v - min_in) / (max_in - min_in) * (max_out - min_out) + min_out;
}

// interpolate using a table, clamping at either end
float clamped_table_interplolation(float v, float table[2][INTERPOLATION_TABLE_LENGTH], int table_length) {
  if (v < table[0][0]) {
    return table[0][0]; // if starting value is below min, return min
  }
  for (int i = 0; i < table_length - 1; i++) {
    if (table[0][i] <= v && v < table[0][i + 1]) {
      return linear_interpolation(v, table[0][i], table[0][i + 1], table[1][i], table[1][i + 1]);
    }
  }
  return table[1][table_length - 1]; // if starting value is above max, return max
}
#pragma endregion

// find CF in terms of thrust using linear interpolation
float cf(float thrust) {
  return clamped_table_interplolation(thrust, cf_thrust_table, CF_THRUST_TABLE_LEN);
}

// convert thrust to chamber pressure
// https://seitzman.gatech.edu/classes/ae4451/thrust_coefficient.pdf
// OUTPUT: chamber pressure
float chamber_pressure(float thrust) {
  return thrust / cf(thrust) / tadpole_AREA_OF_THROAT;
}

// convert chamber pressure to total mass flow rate
// https://en.wikipedia.org/wiki/Characteristic_velocity
// OUTPUT: total mass flow rate
float mass_flow_rate(float chamber_pressure) {
  return chamber_pressure * tadpole_AREA_OF_THROAT / tadpole_C_STAR * GRAVITY_FT_S;
}

// convert total mass flow into OX and FUEL flow rates
// OUTPUT: mass_flow_ox and mass_flow_fuel
void mass_balance(float total_mass_flow, float *mass_flow_ox, float *mass_flow_fuel) {
  *mass_flow_ox = total_mass_flow / (1 + tadpole_MASS_FLOW_RATIO) * tadpole_MASS_FLOW_RATIO;
  *mass_flow_fuel = total_mass_flow / (1 + tadpole_MASS_FLOW_RATIO);
}

// convert mass_flow into pressure
// OUTPUT: pressure (psi)
float cavitating_venturi(float mass_flow, fluid cvFluid) {
  return pow(mass_flow / cvFluid.cav_vent_throat_area / cvFluid.cav_vent_cd, 2) / 2 / GRAVITY_IN_S / cvFluid.density + cvFluid.vapour_pressure;
}

// convert mass_flow into valve cv
// OUTPUT: cv (unitless)
float sub_critical_cv(float mass_flow, float downstream_pressure, fluid cvFluid) {
  return mass_flow / sqrt((cvFluid.upstream_pressure - downstream_pressure) * cvFluid.specific_gravity);
}

// convert cv into valve angle
// OUTPUT: valve angle (radians)
float valve_angle(float cv) {
  return clamped_table_interplolation(cv, valve_angle_table, VALVE_ANGLE_TABLE_LEN);
} // TODO - convert this to radians

// get valve angles given thrust
void open_loop_thrust_control(float thrust, float *angle_ox, float *angle_fuel) {
  float mass_flow_ox;
  float mass_flow_fuel;
  mass_balance(mass_flow_rate(chamber_pressure(thrust)), &mass_flow_ox, &mass_flow_fuel);
  *angle_ox = valve_angle(sub_critical_cv(mass_flow_ox, cavitating_venturi(mass_flow_ox, ox), ox));
  *angle_fuel = valve_angle(sub_critical_cv(mass_flow_fuel, cavitating_venturi(mass_flow_fuel, ipa), ipa));
}
