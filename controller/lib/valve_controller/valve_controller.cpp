#include "valve_controller.h"
#include "pi_controller.h"
#include "math.h"

#define tadpole_AREA_OF_THROAT 1.69 // in^2
#define tadpole_C_STAR 4998.0654    // ft / s // TODO RJN OL - replace with data from testing
#define tadpole_MASS_FLOW_RATIO 1.2 // #ox = 1.2 * ipa
#define GRAVITY_FT_S 32.1740        // Gravity in (ft / s^2)

#define IN3_TO_GAL 0.004329     // convert cubic inches to gallons
#define PER_SEC_TO_PER_MIN 60   // convert per second to per minute
#define DENSITY_WATER 0.0360724 // lb/in^3

#define INTERPOLATION_TABLE_LENGTH 30 // max length of all tables - set to enable passing tables to functions

#define CF_THRUST_TABLE_LEN 2 // TODO RJN OL - replace with data from testing
// thrust (lbf) to cf (unitless)
float cf_thrust_table[2][INTERPOLATION_TABLE_LENGTH] = {
    {220, 550},
    {1.12, 1.3}};

#define OX_DENSITY_TABLE_LEN 20
// temperature (K) to density (lb/in^3)
float ox_density_table[2][INTERPOLATION_TABLE_LENGTH] = {
    {55, 60, 65, 70, 75, 80, 85, 90, 95, 100, 105, 110, 115, 120, 125, 130, 135, 140, 145, 150},
    {0.04709027778, 0.04631539352, 0.04550925926, 0.0446880787, 0.04385474537, 0.04300810185, 0.04214525463, 0.04126099537, 0.04035127315, 0.03941087963, 0.03843229167, 0.03740856481, 0.03632986111, 0.03518287037, 0.03394965278, 0.03260416667, 0.03110532407, 0.02938020833, 0.0272806713, 0.02440335648}};

#define OX_PRESSURE_TABLE_LEN 20
// temperature (K) to vapour pressure (psi)
float ox_pressure_table[2][INTERPOLATION_TABLE_LENGTH] = {
    {55, 60, 65, 70, 75, 80, 85, 90, 95, 100, 105, 110, 115, 120, 125, 130, 135, 140, 145, 150},
    {0.0259, 0.10527, 0.33866, 0.90826, 2.1099, 4.369, 8.2426, 14.41, 23.653, 36.84, 54.901, 78.814, 109.59, 148.27, 195.93, 253.68, 322.72, 404.33, 500.05, 611.86}};

#define OX_MANIFOLD_TABLE_LEN 21
// thrust (lbf) to pressure (psi)
float ox_manifold_table[2][INTERPOLATION_TABLE_LENGTH] = {
    {192.5, 210.556, 228.611, 246.667, 264.722, 282.778, 300.833, 318.889, 336.944, 355.0, 373.056, 391.111, 409.167, 427.222, 445.278, 463.333, 481.389, 499.444, 517.5, 535.556, 550.0},
    {118.055, 127.34, 136.64, 146.082, 155.44, 165.182, 175.069, 184.896, 194.888, 205.049, 215.378, 225.702, 236.381, 246.93, 257.695, 268.683, 279.647, 290.878, 301.999, 313.482, 322.79}};

#define IPA_MANIFOLD_TABLE_LEN 21
// thrust (lbf) to pressure (psi)
float ipa_manifold_table[2][INTERPOLATION_TABLE_LENGTH] = {
    {192.5, 210.556, 228.611, 246.667, 264.722, 282.778, 300.833, 318.889, 336.944, 355.0, 373.056, 391.111, 409.167, 427.222, 445.278, 463.333, 481.389, 499.444, 517.5, 535.556, 550.0},
    {120.844, 130.535, 140.262, 150.16, 159.99, 170.243, 180.67, 191.054, 201.631, 212.408, 223.382, 234.368, 245.752, 257.017, 268.529, 280.3, 292.062, 304.129, 316.095, 328.466, 338.509}};

Venturi ox_venturi{.inlet_area = 0.127, .throat_area = 0.0204, .cd = 1};  // in^2 for both // TODO - update!
Venturi ipa_venturi{.inlet_area = 0.127, .throat_area = 0.0204, .cd = 1}; // in^2 for both

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

// get oxygen properties using temperature in Kelvin
float ox_density_from_temperature(float temperature) {
  return clamped_table_interplolation(temperature, ox_density_table, OX_DENSITY_TABLE_LEN);
}

// get ipa properties using temperature in Kelvin
float ipa_density() {
  return 0.02836; // lb/in^3
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

// convert total mass flow into OX and IPA flow rates
// INPUT: total_mass_flow (lbm/s)
// OUTPUT: mass_flow_ox (lbm/s) and mass_flow_ipa (lbm/s)
void mass_balance(float total_mass_flow, float *mass_flow_ox, float *mass_flow_ipa) {
  *mass_flow_ox = total_mass_flow / (1 + tadpole_MASS_FLOW_RATIO) * tadpole_MASS_FLOW_RATIO;
  *mass_flow_ipa = total_mass_flow / (1 + tadpole_MASS_FLOW_RATIO);
}

// convert mass_flow into valve flow coefficient (cv)
// OUTPUT: valve flow coefficient (assume this is unitless)
// INPUT: mass_flow (lbm/s), downstream pressure (psi), fluid properties
float sub_critical_cv(float mass_flow, float upstream_pressure, float downstream_pressure, float density) {
  float pressure_delta = upstream_pressure - downstream_pressure; // TODO - update for cold flows
  pressure_delta = pressure_delta > 0 ? pressure_delta : 0.0001;  // block negative under sqrt and divide by 0
  return mass_flow * IN3_TO_GAL * PER_SEC_TO_PER_MIN * sqrt(1 / (pressure_delta * density * DENSITY_WATER));
}

// Lookup the valve angle using linear interpolation
// INPUT: valve flow coefficient (assume this is unitless)
// OUTPUT: valve angle (degrees)
#define LOX_ALPHA 2.5
#define LOX_BETA 58
#define LOX_GAMMA 11
float lox_valve_angle(float cv) {
  return -LOX_GAMMA * log(LOX_ALPHA / cv - 1) + LOX_BETA;
}

#define IPA_ALPHA 2.95
#define IPA_BETA 63
#define IPA_GAMMA 10
float ipa_valve_angle(float cv) {
  return -IPA_GAMMA * log(IPA_ALPHA / cv - 1) + IPA_BETA;
}
// Lookup ox manifold pressure using linear interpolation
// INPUT: thrust
// OUTPUT: ox manifold pressure (psi)
float ox_manifold_pressure(float thrust) {
  return clamped_table_interplolation(thrust, ox_manifold_table, OX_MANIFOLD_TABLE_LEN);
}

// Lookup ipa manifold pressure using linear interpolation
// INPUT: thrust
// OUTPUT: ipa manifold pressure (psi)
float ipa_manifold_pressure(float thrust) {
  return clamped_table_interplolation(thrust, ipa_manifold_table, IPA_MANIFOLD_TABLE_LEN);
}

// Estimates mass flow across a venturi using pressure sensor data and fluid information.
float estimate_mass_flow(Fluid_Line fluid_line, Venturi venturi, float fluid_density) {
  float pressure_delta = fluid_line.venturi_differential_pressure;
  pressure_delta = pressure_delta > 0 ? pressure_delta : 0; // block negative under sqrt
  float area_term = pow(venturi.throat_area / venturi.inlet_area, 2);
  return venturi.throat_area * sqrt(2 * fluid_density * pressure_delta * 12 * GRAVITY_FT_S / (1 - area_term)) * venturi.cd;
}

VC_State vc_state;
// get valve angles (degrees) given thrust (lbf) and current sensor data
void open_loop_thrust_control(float thrust, Sensor_Data sensor_data, float *angle_ox, float *angle_ipa) {
  float measured_mass_flow_ox = estimate_mass_flow(sensor_data.ox, ox_venturi, ox_density_from_temperature(sensor_data.ox.venturi_temperature));
  float measured_mass_flow_ipa = estimate_mass_flow(sensor_data.ipa, ipa_venturi, ipa_density());

  float mass_flow_total = mass_flow_rate(chamber_pressure(thrust));
  float mass_flow_ox;
  float mass_flow_ipa;
  mass_balance(mass_flow_total, &mass_flow_ox, &mass_flow_ipa);

  float ox_valve_downstream_pressure_goal = ox_manifold_pressure(thrust); // TODO RJN OL - multiply these by coeff / replace with venturi upstream?
  float ipa_valve_downstream_pressure_goal = ipa_manifold_pressure(thrust);

  *angle_ox = lox_valve_angle(sub_critical_cv(mass_flow_ox, sensor_data.ox.valve_upstream_pressure, ox_valve_downstream_pressure_goal, ox_density_from_temperature(sensor_data.ox.valve_temperature)));
  *angle_ipa = ipa_valve_angle(sub_critical_cv(mass_flow_ipa, sensor_data.ipa.valve_upstream_pressure, ipa_valve_downstream_pressure_goal, ipa_density()));

  vc_state.ol_lox_mdot = mass_flow_ox;
  vc_state.ol_ipa_mdot = mass_flow_ipa;
  vc_state.measured_lox_mdot = measured_mass_flow_ox;
  vc_state.measured_ipa_mdot = measured_mass_flow_ipa;
  vc_state.ol_lox_angle = *angle_ox;
  vc_state.ol_ipa_angle = *angle_ipa;
}

// get valve angles (degrees) given thrust (lbf) and current sensor data using PID controllers
void closed_loop_thrust_control(float thrust, Sensor_Data sensor_data, float *angle_ox, float *angle_ipa) {
  // ol_ for open loop computations
  // err_ for err between ol and sensor
  // col_ for closed loop computation

  float measured_mass_flow_ox = estimate_mass_flow(sensor_data.ox, ox_venturi, ox_density_from_temperature(sensor_data.ox.venturi_temperature));
  float measured_mass_flow_ipa = estimate_mass_flow(sensor_data.ipa, ipa_venturi, ipa_density());

  float ol_chamber_pressure = chamber_pressure(thrust);
  float err_chamber_pressure = sensor_data.chamber_pressure - ol_chamber_pressure;
  float ol_mdot_total = mass_flow_rate(ol_chamber_pressure);
  float cl_mdot_total = ol_mdot_total - ClosedLoopControllers::Chamber_Pressure_Controller.compute(err_chamber_pressure);

  float ol_mass_flow_ox;
  float ol_mass_flow_ipa;
  mass_balance(cl_mdot_total, &ol_mass_flow_ox, &ol_mass_flow_ipa);

  float err_mass_flow_ox = measured_mass_flow_ox - ol_mass_flow_ox;
  float err_mass_flow_ipa = measured_mass_flow_ipa - ol_mass_flow_ipa;

  float ox_valve_downstream_pressure_goal = ox_manifold_pressure(thrust);
  float ipa_valve_downstream_pressure_goal = ipa_manifold_pressure(thrust);

  float ol_angle_ox = lox_valve_angle(sub_critical_cv(ol_mass_flow_ox, sensor_data.ox.valve_upstream_pressure, ox_valve_downstream_pressure_goal, ox_density_from_temperature(sensor_data.ox.valve_temperature)));
  float ol_angle_ipa = ipa_valve_angle(sub_critical_cv(ol_mass_flow_ipa, sensor_data.ipa.valve_upstream_pressure, ipa_valve_downstream_pressure_goal, ipa_density()));

  *angle_ox = ol_angle_ox - ClosedLoopControllers::LOX_Angle_Controller.compute(err_mass_flow_ox);
  *angle_ipa = ol_angle_ipa - ClosedLoopControllers::IPA_Angle_Controller.compute(err_mass_flow_ipa);

  vc_state.ol_lox_mdot = ol_mass_flow_ox;
  vc_state.ol_ipa_mdot = ol_mass_flow_ipa;
  vc_state.measured_lox_mdot = measured_mass_flow_ox;
  vc_state.measured_ipa_mdot = measured_mass_flow_ipa;
  vc_state.ol_lox_angle = ol_angle_ox;
  vc_state.ol_ipa_angle = ol_angle_ipa;
}

void log_only(Sensor_Data sensor_data) {
  float measured_mass_flow_ox = estimate_mass_flow(sensor_data.ox, ox_venturi, ox_density_from_temperature(sensor_data.ox.venturi_temperature));
  float measured_mass_flow_ipa = estimate_mass_flow(sensor_data.ipa, ipa_venturi, ipa_density());

  vc_state.ol_lox_mdot = 0;
  vc_state.ol_ipa_mdot = 0;
  vc_state.measured_lox_mdot = measured_mass_flow_ox;
  vc_state.measured_ipa_mdot = measured_mass_flow_ipa;
  vc_state.ol_lox_angle = 0;
  vc_state.ol_ipa_angle = 0;
}