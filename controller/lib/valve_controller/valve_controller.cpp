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

#define OX_INJ_AREA 0.0498   // in^2
#define IPA_INJ_AREA 0.04031 // in^2
#define OX_INJ_CD 0.35       // TODO RJN - change during hotfires
#define IPA_INJ_CD 0.54

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

#define IPA_CV_TABLE_LEN 16
float ipa_valve_cv_table[2][INTERPOLATION_TABLE_LENGTH] = {
    {0, 0.0807, 0.0812, 0.0812, 0.0812, 0.1185, 0.2065, 0.3559, 0.5746, 0.8635, 1.2144, 1.6087, 2.0153, 2.3885, 2.6666, 2.7699},
    {0, 6.0000, 12.0000, 18.0000, 24.0000, 30.0000, 36.0000, 42.0000, 48.0000, 54.0000, 60.0000, 66.0000, 72.0000, 78.0000, 84.0000, 90.0000}};

Venturi ox_venturi{.inlet_area = 0.127, .throat_area = 0.066, .cd = 1};  // in^2 for both
Venturi ipa_venturi{.inlet_area = 0.127, .throat_area = 0.062, .cd = 1}; // in^2 for both

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
  float pressure_delta = upstream_pressure - downstream_pressure;
  pressure_delta = pressure_delta > 0 ? pressure_delta : 0.0001; // block negative under sqrt and divide by 0
  return mass_flow * IN3_TO_GAL * PER_SEC_TO_PER_MIN * sqrt(1 / (pressure_delta * density * DENSITY_WATER));
}

// Lookup the valve angle using linear interpolation
// INPUT: valve flow coefficient (assume this is unitless)
// OUTPUT: valve angle (degrees)
float ipa_valve_angle(float cv) {
  return clamped_table_interplolation(cv, ipa_valve_cv_table, IPA_CV_TABLE_LEN);
}

float manifold_drop(float target_mass_flow, float density, float injector_area, float c_d) {
  return pow(target_mass_flow, 2) / (2 * density * GRAVITY_FT_S * 12 * pow(c_d * injector_area, 2));
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

  float mass_flow_ipa = thrust;
  float ipa_manifold_drop = manifold_drop(mass_flow_ipa, ipa_density(), IPA_INJ_AREA, IPA_INJ_CD);
  float ipa_valve_downstream_pressure_goal = 14.7 + ipa_manifold_drop;

  *angle_ox = 50;
  *angle_ipa = ipa_valve_angle(sub_critical_cv(mass_flow_ipa, sensor_data.ipa.valve_upstream_pressure, ipa_valve_downstream_pressure_goal, ipa_density()));

  vc_state.ol_lox_mdot = 0;
  vc_state.ol_ipa_mdot = mass_flow_ipa;
  vc_state.measured_lox_mdot = measured_mass_flow_ox;
  vc_state.measured_ipa_mdot = measured_mass_flow_ipa;
  vc_state.ol_lox_angle = *angle_ox;
  vc_state.ol_ipa_angle = *angle_ipa;
  vc_state.ox_valve_downstream_calc = 0;
  vc_state.ipa_valve_downstream_calc = ipa_valve_downstream_pressure_goal;
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

  float ox_manifold_drop = manifold_drop(ol_mass_flow_ox, ox_density_from_temperature(sensor_data.ox.valve_temperature), OX_INJ_AREA, OX_INJ_CD);
  float ipa_manifold_drop = manifold_drop(ol_mass_flow_ipa, ipa_density(), IPA_INJ_AREA, IPA_INJ_CD);
  float ox_valve_downstream_pressure_goal = 14.7 + ox_manifold_drop; // TODO RJN - change to chamber pressure for hotfires
  float ipa_valve_downstream_pressure_goal = 14.7 + ipa_manifold_drop;

  float ol_angle_ox = 50;
  float ol_angle_ipa = ipa_valve_angle(sub_critical_cv(ol_mass_flow_ipa, sensor_data.ipa.valve_upstream_pressure, ipa_valve_downstream_pressure_goal, ipa_density()));

  *angle_ox = ol_angle_ox - ClosedLoopControllers::LOX_Angle_Controller.compute(err_mass_flow_ox);
  *angle_ipa = ol_angle_ipa - ClosedLoopControllers::IPA_Angle_Controller.compute(err_mass_flow_ipa);

  vc_state.ol_lox_mdot = ol_mass_flow_ox;
  vc_state.ol_ipa_mdot = ol_mass_flow_ipa;
  vc_state.measured_lox_mdot = measured_mass_flow_ox;
  vc_state.measured_ipa_mdot = measured_mass_flow_ipa;
  vc_state.ol_lox_angle = ol_angle_ox;
  vc_state.ol_ipa_angle = ol_angle_ipa;
  vc_state.ox_valve_downstream_calc = ox_valve_downstream_pressure_goal;
  vc_state.ipa_valve_downstream_calc = ipa_valve_downstream_pressure_goal;
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
  vc_state.ox_valve_downstream_calc = 0;
  vc_state.ipa_valve_downstream_calc = 0;
}