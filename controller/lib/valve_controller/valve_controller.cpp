#include <valve_controller.hpp>

#define INTERPOLATION_TABLE_LENGTH 20

float valve_angle_table[2][INTERPOLATION_TABLE_LENGTH] = {
    {0.000, 0.070, 0.161, 0.378, 0.670, 1.000, 1.450, 2.050, 2.780, 3.710, 4.960},
    {0, 9, 18, 27, 36, 45, 54, 63, 72, 81, 90}};

// TODO - efficiency coeff?
#define tadpole_AREA_OF_THROAT 1.69 // in^2
#define tadpole_C_STAR 4998.0654    // ft / s
#define tadpole_C_F 1.12            // unitless - TODO - why does this vary with thrust?
#define tadpole_MASS_FLOW_RATIO 1.2 // #ox = 1.2 * fuel
#define GRAVITY_FT_S 32.1740        // Gravity in ft / s^2
#define LIQUID_SPECIFIC_GRAVITY 1.5 // TODO - set this
#define UPSTREAM_PRESSURE 1000      // psi       // TODO - unit conversions // needs to be user configurable

// CD * area of throat for cav venturi

// liquid specific
// LIQUID_SPECIFIC_GRAVITY
// vapour pressure
// density

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

// convert chamber pressure to total mass flow rate
// https://en.wikipedia.org/wiki/Characteristic_velocity
// OUTPUT: total mass flow rate
float mass_flow_rate(float chamber_pressure) {
  return chamber_pressure * tadpole_AREA_OF_THROAT / tadpole_C_STAR * GRAVITY_FT_S;
}

// find CF in terms of thrust using linear interpolation
float cf(float thrust) {
  return (thrust - 40) / 60 * (1.3 - 1.12) + 1.12;
}

// convert thrust to chamber pressure
// https://seitzman.gatech.edu/classes/ae4451/thrust_coefficient.pdf
// OUTPUT: chamber pressure
float chamber_pressure(float thrust) {
  return thrust / tadpole_C_F / tadpole_AREA_OF_THROAT;
}

// convert total mass flow into OX and FUEL flow rates
// OUTPUT: mass_flow_ox and mass_flow_fuel
void mass_balance(float total_mass_flow, float *mass_flow_ox, float *mass_flow_fuel) {
  *mass_flow_ox = total_mass_flow / (1 + tadpole_MASS_FLOW_RATIO) * tadpole_MASS_FLOW_RATIO;
  *mass_flow_fuel = total_mass_flow / (1 + tadpole_MASS_FLOW_RATIO);
}

// convert cv into valve angle
// OUTPUT: valve angle (radians)
float valve_angle(float cv) {
  return clamped_table_interplolation(cv, valve_angle_table, 11);
} // TODO - convert this to radians

// convert mass_flow into valve cv
// OUTPUT: cv (unitless)
float sub_critical_cv(float mass_flow, float downstream_pressure) {
  return 1.17 * mass_flow / sqrt(UPSTREAM_PRESSURE - downstream_pressure * LIQUID_SPECIFIC_GRAVITY);
}

// get valve angles given thrust
void open_loop_thrust_control(float thrust, float *angle_ox, float *angle_fuel) {
  float mass_flow_ox;
  float mass_flow_fuel;
  mass_balance(mass_flow_rate(chamber_pressure(thrust)), &mass_flow_ox, &mass_flow_fuel);
  *angle_ox = valve_angle(sub_critical_cv(mass_flow_ox, 0));
  *angle_fuel = valve_angle(sub_critical_cv(mass_flow_fuel, 0));
}