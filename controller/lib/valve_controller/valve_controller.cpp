#include <valve_controller.hpp>
#include <math.h>

// #include <Router.h>
// #include "CString.h"

// TODO - efficiency coeff?
// TODO - update fluid constants based on temperature and pressure
#define tadpole_AREA_OF_THROAT 1.69      // in^2
#define tadpole_C_STAR 4998.0654         // ft / s
#define tadpole_MASS_FLOW_RATIO 1.2      // #ox = 1.2 * fuel
#define GRAVITY_FT_S 32.1740             // Gravity in (ft / s^2)
#define GRAVITY_IN_S (GRAVITY_FT_S * 12) // Gravity in (in / s^2)

#define cav_vent_ox_AREA_OF_THROAT 0.00989 // in^2 #-200 deg C
#define cav_vent_ox_CD 0.975               // unitless
#define fluid_ox_VAPOUR_PRESSURE 14.2      // psi // TODO - depends on temperature
#define fluid_ox_DENSITY 0.04146           // lb/in^3 // TODO - vary based on *temperature* or pressure
#define fluid_ox_UPSTREAM_PRESSURE 820     // psi

#define cav_vent_ipa_AREA_OF_THROAT 0.00989 // in^2 #room temp
#define cav_vent_ipa_CD 0.975               // unitless
#define fluid_ipa_VAPOUR_PRESSURE 0.638     // psi // TODO - depends on temperature
#define fluid_ipa_DENSITY 0.02836           // lb/in^3 // TODO - assume constant
#define fluid_ipa_UPSTREAM_PRESSURE 820     // psi

#define IN3_TO_GAL 0.004329       // convert cubic inches to gallons
#define PER_SEC_TO_PER_MIN 60     // convert per second to per minute
#define LB_TO_TON 0.000453592     // convert lb to metric tons
#define PER_IN3_TO_PER_M3 61023.7 // convert per in^3 to per m^3

#define INTERPOLATION_TABLE_LENGTH 20 // max length of all tables - set to enable passing tables to functions
#define VALVE_ANGLE_TABLE_LEN 11
// CV (assume unitless) to angle (degrees)
float valve_angle_table[2][INTERPOLATION_TABLE_LENGTH] = {
    {0.000, 0.070, 0.161, 0.378, 0.670, 1.000, 1.450, 2.050, 2.780, 3.710, 4.960},
    {0, 9, 18, 27, 36, 45, 54, 63, 72, 81, 90}};

#define CF_THRUST_TABLE_LEN 2
// thrust (lbf) to cf (unitless)
float cf_thrust_table[2][INTERPOLATION_TABLE_LENGTH] = {
    {220, 550},
    {1.12, 1.3}};

struct cav_vent {
  float cav_vent_throat_area;
  float cav_vent_cd;
};

struct fluid {
  float vapour_pressure;
  float density;
};

cav_vent ox_cv{cav_vent_ox_AREA_OF_THROAT, cav_vent_ox_CD};
cav_vent ipa_cv{cav_vent_ipa_AREA_OF_THROAT, cav_vent_ipa_CD};
fluid ox{fluid_ox_VAPOUR_PRESSURE, fluid_ox_DENSITY};
fluid ipa{fluid_ipa_VAPOUR_PRESSURE, fluid_ipa_DENSITY};

// maps v from (min_in, max_in) to (min_out, max_out)
float linear_interpolation(float v, float min_in, float max_in, float min_out, float max_out) {
  return (v - min_in) / (max_in - min_in) * (max_out - min_out) + min_out;
}

// linearly interpolate using the 2 nearest values in a table
// first row of table represents input
// second row of table represents output
// if value is lower than the first value or larger than the last value, clamp to the largest or smallest output
float clamped_table_interplolation(float v, float table[2][INTERPOLATION_TABLE_LENGTH], int table_length) {
  if (v < table[0][0]) {
    return table[1][0]; // if starting value is below min, return min
  }
  for (int i = 0; i < table_length - 1; i++) {
    if (table[0][i] <= v && v < table[0][i + 1]) {
      return linear_interpolation(v, table[0][i], table[0][i + 1], table[1][i], table[1][i + 1]);
    }
  }
  return table[1][table_length - 1]; // if starting value is above max, return max
}

// The thrust coefficient (Cf) varies based on thrust
// Lookup the thrust coefficient using linear interpolation
// INPUT: thrust (lbf)
// OUTPUT: thrust coefficient (unitless)
float cf(float thrust) {
  return clamped_table_interplolation(thrust, cf_thrust_table, CF_THRUST_TABLE_LEN);
}

// convert thrust to chamber pressure using Cf equation
// INPUT: thrust (lbf)
// OUTPUT: chamber pressure (psi)
float chamber_pressure(float thrust) {
  return thrust / cf(thrust) / tadpole_AREA_OF_THROAT;
}

// convert chamber pressure to total mass flow rate using c* equation
// INPUT: chamber pressure (psi)
// OUTPUT: total mass flow rate (lbm/s)
float mass_flow_rate(float chamber_pressure) {
  return chamber_pressure * tadpole_AREA_OF_THROAT / tadpole_C_STAR * GRAVITY_FT_S;
}

// convert total mass flow into OX and FUEL flow rates
// INPUT: total_mass_flow (lbm/s)
// OUTPUT: mass_flow_ox (lbm/s) and mass_flow_fuel (lbm/s)
void mass_balance(float total_mass_flow, float *mass_flow_ox, float *mass_flow_fuel) {
  *mass_flow_ox = total_mass_flow / (1 + tadpole_MASS_FLOW_RATIO) * tadpole_MASS_FLOW_RATIO;
  *mass_flow_fuel = total_mass_flow / (1 + tadpole_MASS_FLOW_RATIO);
}

// convert mass_flow into pressure using cavitating venturi equation
// INPUT: mass_flow (lbm/s), fluid properties
// OUTPUT: pressure (psi)
float cavitating_venturi(float mass_flow, cav_vent cvProperties, fluid cvFluid) {
  return pow(mass_flow / cvProperties.cav_vent_throat_area / cvProperties.cav_vent_cd, 2) / 2 / GRAVITY_IN_S / cvFluid.density + cvFluid.vapour_pressure;
}

// convert mass_flow into valve flow coefficient (cv)
// OUTPUT: valve flow coefficient (assume this is unitless)
// INPUT: mass_flow (lbm/s), downstream pressure (psi), fluid properties
float sub_critical_cv(float mass_flow, float upstream_pressure, float downstream_pressure, fluid cvFluid) {
  return mass_flow * IN3_TO_GAL * PER_SEC_TO_PER_MIN * sqrt(LB_TO_TON * PER_IN3_TO_PER_M3 / (upstream_pressure - downstream_pressure) / cvFluid.density);
}

// Lookup the valve angle using linear interpolation
// INPUT: valve flow coefficient (assume this is unitless)
// OUTPUT: valve angle (degrees)
float valve_angle(float cv) {
  return clamped_table_interplolation(cv, valve_angle_table, VALVE_ANGLE_TABLE_LEN);
}

// get valve angles (degrees) given thrust
void open_loop_thrust_control(float thrust, float *angle_ox, float *angle_fuel) {
  float mass_flow_ox;
  float mass_flow_fuel;
  mass_balance(mass_flow_rate(chamber_pressure(thrust)), &mass_flow_ox, &mass_flow_fuel);

  float ox_valve_downstream_pressure_goal = cavitating_venturi(mass_flow_ox, ox_cv, ox);
  float ipa_valve_downstream_pressure_goal = cavitating_venturi(mass_flow_fuel, ipa_cv, ipa);

  *angle_ox = valve_angle(sub_critical_cv(mass_flow_ox, fluid_ox_UPSTREAM_PRESSURE, ox_valve_downstream_pressure_goal, ox));
  *angle_fuel = valve_angle(sub_critical_cv(mass_flow_fuel, fluid_ipa_UPSTREAM_PRESSURE, ipa_valve_downstream_pressure_goal, ipa));
}

// void update_float_value(float *value, const char *prompt) {
//   Router::info(prompt);
//   String data_str = Router::read(50); // int buffer size
//   Router::info("Response: " + data_str);
//   float new_value;
//   int result = std::sscanf(data_str.c_str(), "%f", &new_value);
//   if (result != 1) {
//     Router::info("Could not convert input to a float, not continuing");
//     return;
//   }
//   *value = new_value;
// }

// // updates ox pressure (in psi)
// void set_ox_pressure(float pressure) { // TODO - replace these with "lambda" functions
//   update_float_value(&ox.upstream_pressure, "Ox Pressure (psi): ");
// }

// // updates ipa pressure (in psi)
// void set_ipa_pressure(float pressure) {
//   update_float_value(&ipa.upstream_pressure, "IPA Pressure (psi): ");
// }

// // updates ox temperature (in K)
// void set_ox_temperature(float temperature) { // TODO - lookup table updates vapour pressure
// }

// // updates ipa temperature (in K)
// void set_ipa_temperature(float temperature) { // TODO - lookup table updates vapour pressure
// }

// CString<200> fluid_info;
// void print_fluid_info() {
//   Router::info("OX Info:");
//   fluid_info.clear();
//   fluid_info << "CV Throat Area (in^2): " << ox.cav_vent_throat_area << "\n"
//              << "CV CD (unitless): " << ox.cav_vent_cd << "\n"
//              << "Vapour Pressure (psi): " << ox.vapour_pressure << "\n"
//              << "Density (lb / in^3): " << ox.density << "\n"
//              << "Upstream Pressure (psi): " << ox.upstream_pressure << "\n";
//   Router::info(fluid_info.str);
//   Router::info("\n");

//   Router::info("IPA Info:");
//   fluid_info.clear();
//   fluid_info << "CV Throat Area (in^2): " << ipa.cav_vent_throat_area << "\n"
//              << "CV CD (unitless): " << ipa.cav_vent_cd << "\n"
//              << "Vapour Pressure (psi): " << ipa.vapour_pressure << "\n"
//              << "Density (lb / in^3): " << ipa.density << "\n"
//              << "Upstream Pressure (psi): " << ipa.upstream_pressure << "\n";
//   Router::info(fluid_info.str);
//   Router::info("\n");
// }