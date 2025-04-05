#include "valve_controller.h"
#include "pi_controller.h"
#include "math.h"

// #define tadpole_AREA_OF_THROAT 1.69 // in^2
// #define tadpole_C_STAR 4998.0654    // ft / s // TODO RJN OL - replace with data from testing
// #define tadpole_MASS_FLOW_RATIO 1.2 // #ox = 1.2 * ipa
#define GRAVITY_FT_S 32.1740 // Gravity in (ft / s^2)

#define IN3_TO_GAL 0.004329       // convert cubic inches to gallons
#define PER_SEC_TO_PER_MIN 60     // convert per second to per minute
#define LB_TO_TON 0.000453592     // convert lb to metric tons
#define PER_IN3_TO_PER_M3 61023.7 // convert per in^3 to per m^3

#define INTERPOLATION_TABLE_LENGTH 30 // max length of all tables - set to enable passing tables to functions
#define VALVE_ANGLE_TABLE_LEN 11
// CV (assume unitless) to angle (degrees)
float valve_angle_table[2][INTERPOLATION_TABLE_LENGTH] = {
    {0.000, 0.070, 0.161, 0.378, 0.670, 1.000, 1.450, 2.050, 2.780, 3.710, 4.960},
    {0, 9, 18, 27, 36, 45, 54, 63, 72, 81, 90}};

Sensor_Data default_sensor_data{
    .water = {
        .tank_pressure = 820,           // psi
        .venturi_upstream_pressure = 0, // psi - not needed for OL
        .venturi_throat_pressure = 0,   // psi - not needed for OL
        .temperature = 0,               // K
    }};

Venturi water_venturi{.inlet_area = 0.127, .throat_area = 0.0204, .cd = 0.8}; // in^2 for both

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

float water_density() {
  return 0.0360724; // lb/in^3
}

// convert mass_flow into valve flow coefficient (cv)
// OUTPUT: valve flow coefficient (assume this is unitless)
// INPUT: mass_flow (lbm/s), downstream pressure (psi), fluid properties
float sub_critical_cv(float mass_flow, float upstream_pressure, float downstream_pressure, float density) {
  float pressure_delta = upstream_pressure - downstream_pressure;
  pressure_delta = pressure_delta > 0 ? pressure_delta : 0.0001; // block negative under sqrt and divide by 0
  return mass_flow * IN3_TO_GAL * PER_SEC_TO_PER_MIN * sqrt(LB_TO_TON * PER_IN3_TO_PER_M3 / pressure_delta / density);
}

// Lookup the valve angle using linear interpolation
// INPUT: valve flow coefficient (assume this is unitless)
// OUTPUT: valve angle (degrees)
float valve_angle(float cv) {
  return clamped_table_interplolation(cv, valve_angle_table, VALVE_ANGLE_TABLE_LEN);
}

FF_State ff_state;

// Estimates mass flow across a venturi using pressure sensor data and fluid information.
float estimate_mass_flow(Fluid_Line fluid_line, Venturi venturi, float fluid_density) {
  float pressure_delta = fluid_line.venturi_upstream_pressure - fluid_line.venturi_throat_pressure;
  pressure_delta = pressure_delta > 0 ? pressure_delta : 0; // block negative under sqrt
  float area_term = 1 - pow(venturi.throat_area / venturi.inlet_area, 2);
  return venturi.throat_area * sqrt(2 * fluid_density * pressure_delta * GRAVITY_FT_S / (1 - area_term)) * venturi.cd;
}

// get valve angles (degrees) given thrust (lbf) and current sensor data
float open_loop_water_flow(float mass_flow_water, Sensor_Data sensor_data) {
  float downstream_pressure_goal = 14.3;
  float angle_water = valve_angle(sub_critical_cv(mass_flow_water, sensor_data.water.tank_pressure, downstream_pressure_goal, water_density()));
  ff_state.mass_flow_estimate = -1;
  ff_state.ol_water_angle = angle_water;
  return angle_water;
}

// get valve angles (degrees) given thrust (lbf) and current sensor data
float closed_loop_water_flow(float mass_flow_water, Sensor_Data sensor_data) {
  float downstream_pressure_goal = 14.3;
  float mass_flow_estimate = estimate_mass_flow(sensor_data.water, water_venturi, water_density());

  float err_mass_flow_water = mass_flow_estimate - mass_flow_water;
  float ol_angle_water = valve_angle(sub_critical_cv(mass_flow_water, sensor_data.water.tank_pressure, downstream_pressure_goal, water_density()));
  ff_state.mass_flow_estimate = mass_flow_estimate;
  ff_state.ol_water_angle = ol_angle_water;
  return ol_angle_water - ClosedLoopControllers::Water_Angle_Controller.compute(err_mass_flow_water);
}